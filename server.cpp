#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <vector>
#include <string>

#include "http.h"

#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 1000
#define MAX_EVENTS 64

using namespace std;

map<int, conn_state> conn_state_map;

const char* inet_ntop2(void *addr, char *buf, size_t size) {
    struct sockaddr_storage *sas = (sockaddr_storage*)addr;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;
    void *src;
    switch (sas->ss_family) {
        case AF_INET:
            sa4 = (sockaddr_in*)addr;
            src = &(sa4->sin_addr);
            break;
        case AF_INET6:
            sa6 = (sockaddr_in6*)addr;
            src = &(sa6->sin6_addr);
            break;
        default:
            return NULL;
    }
    return inet_ntop(sas->ss_family, src, buf, size);
}

int set_socket_to_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);

    if(flags == -1) {
        cerr<<"Error getting flags of the socket"<<endl;
        return -1;
    }
    flags = flags | O_NONBLOCK;

    if(fcntl(sockfd, F_SETFL, flags) == -1) { 
        cerr<<"Error setting the socket to non-blocking"<<endl;
        return -1;
    }

    return 1;
}

int get_listener_socket() {
    int listener;
    int yes = 1;
    int rv;
    struct addrinfo hints, *ai, *p; 

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listener < 0) {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if(bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    if(p == NULL) {
        return -1;
    }

    freeaddrinfo(ai);

    if(listen(listener, SOMAXCONN) == -1) {
        return -1;
    }
    return listener;

}
void close_connection(int epfd, int fd, map<int, conn_state>& conn_state_map) {
    if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        perror("epoll_ctl: deletion");
    }
    conn_state_map.erase(fd);
    close(fd);
}

void handle_new_connections(int epfd, int listener, map<int, conn_state> &conn_state_map) {
    while(true) {
        struct sockaddr_storage remoteaddr; //client addr
        socklen_t addrlen;
        int newfd;
        char remoteIP[INET6_ADDRSTRLEN];

        addrlen = sizeof(remoteaddr);

        newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
        if(newfd == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("accept");
            break;
        }
        if(set_socket_to_non_blocking(newfd) == -1) {
            cerr<<"Failed to set the socket to non blocking"<<endl;
            close(newfd);
            continue;
        }

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = newfd;
        if(epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &ev) == -1) {
            perror("epoll_ctl: addition");
            close(newfd);
            continue;
        }
        conn_state_map[newfd] = {"", 0, string::npos, string::npos, "", 0,connection_state::READING};
    }
}

void handle_recv(int epfd, int fd, map<int, conn_state> &conn_state_map) {
    char temp_buf[MAXDATASIZE];

    conn_state &fd_state = conn_state_map[fd];

    while(true) {
        int numbytes = recv(fd, temp_buf, sizeof(temp_buf) - 1, 0);
        
        if(numbytes == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }            
            close_connection(epfd, fd, conn_state_map);
            return;
        }
        if(numbytes == 0) {
            close_connection(epfd, fd, conn_state_map);
            return;
        };

        fd_state.recv_buf.append(temp_buf, numbytes);
    }

    //parse the buffer
    if(fd_state.header_end == string::npos) {
        size_t search_start;

        if(fd_state.parse_offset > 3) {
            search_start = fd_state.parse_offset - 3;
        } else {
            search_start = 0;
        }

        fd_state.header_end = fd_state.recv_buf.find("\r\n\r\n", search_start);
        fd_state.parse_offset = fd_state.recv_buf.size();

        if(fd_state.header_end != string::npos) {
            size_t cl_pos = fd_state.recv_buf.find("Content-Length");
            if(cl_pos != string::npos) {
                size_t val_start = fd_state.recv_buf.find(": ", cl_pos) + 2;
                size_t val_end = fd_state.recv_buf.find("\r\n", val_start);
                int content_length = stoi(fd_state.recv_buf.substr(val_start, val_end - val_start));
                fd_state.content_length = content_length;
            }
        }
    }

    if(fd_state.header_end != string::npos) {
        size_t total_expected;

        total_expected = fd_state.header_end + 4;
        if(fd_state.content_length != string::npos) {
            total_expected += fd_state.content_length;
        }
        
        if(fd_state.recv_buf.size() >= total_expected) {
            fd_state.state = connection_state::SENDING;
            http_request rq = build_request(fd_state.recv_buf);
            http_response rp;
            string path;

            if(path_builder(rq.path, path) == 0) {
                rp = create_response(404, "", "text/html");
            } else {
                string body = html_to_string(path);
                rp = create_response(200, body, infer_content_type(path));
            }
            string response = build_response(rp);
            fd_state.send_buf = response;
            //changing the interest of the pollfd 
            struct epoll_event ev;
            ev.events = EPOLLOUT;
            ev.data.fd = fd;
            if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                perror("epoll_ctl: mod");
            }
        }
    }
}

void handle_send(int epfd, int fd, map<int, conn_state> &conn_state_map) {
    conn_state &fd_state = conn_state_map[fd];

    while(fd_state.send_offset < fd_state.send_buf.size()) {
        size_t remaining = fd_state.send_buf.size() - fd_state.send_offset;
        int sent = send(fd, fd_state.send_buf.c_str() + fd_state.send_offset, remaining, 0);

        if(sent == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            perror("send");

            close_connection(epfd, fd, conn_state_map);
            return;
        }
        
        if(sent == 0) {
            perror("send");
            close_connection(epfd, fd, conn_state_map);
            return;
        }
        fd_state.send_offset += sent;
    }

        
    if(fd_state.send_offset == fd_state.send_buf.size()) {
        close_connection(epfd, fd, conn_state_map);
    }
}

int main() {
    //to handle clients disconnecting
    signal(SIGPIPE, SIG_IGN);
    
    //creating an epoll instance
    int epfd = epoll_create1(0); 
    if(epfd == -1) {
        perror("epoll");
        exit(1);
    }

    int sockfd, newfd;
    sockfd = get_listener_socket();
    if(sockfd == -1) {
        cerr<<"socket creation error"<<endl;
        exit(1);
    }

    if(set_socket_to_non_blocking(sockfd) == -1) {
        cerr<<"failed to set listener to non-blocking"<<endl;
        exit(1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        perror("epoll_ctl: add listener");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS];
   

    while(1) {
        int ready_fds = epoll_wait(epfd, events, MAX_EVENTS, -1);

        if(ready_fds == -1) {
            perror("epoll wait");
            break;
        }
        if(ready_fds == 0) continue;
        for(int i = 0; i < ready_fds; i++) {
            int fd = events[i].data.fd;
            size_t re = events[i].events;

            if(fd == sockfd) {
                handle_new_connections(epfd, sockfd, conn_state_map);
                continue;
            }
            if(re & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
                close_connection(epfd, fd, conn_state_map);
                continue;
            }else if(re & EPOLLIN) {
                handle_recv(epfd, fd, conn_state_map);
            } else if(re & EPOLLOUT) {
                handle_send(epfd, fd, conn_state_map);
            }
        }
    }

    return 0;
}
