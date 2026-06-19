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

#include <vector>
#include <string>

#include "http.h"

#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 1000

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
        cout<<"Poll server"<<gai_strerror(rv)<<endl;
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

    if(listen(listener, 10) == -1) {
        return -1;
    }
    cout<<"Successfully got the listener socket"<<endl;
    return listener;

}

void add_to_pfds(vector<struct pollfd> &pfds, int newfd) {
    struct pollfd pfd;
    cout<<"added to pfds"<<endl;

    pfd.fd = newfd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    pfds.push_back(pfd);

}

void del_from_pfds(vector<struct pollfd> &pfds, int i) {
    cout<<"removed from pfds"<<endl;
    pfds[i] = pfds.back();
    pfds.pop_back();
}

void handle_new_connections(int listener, vector<struct pollfd> &pfds, map<int, conn_state> &conn_state_map) {
    cout<<"Handling a new connection"<<endl;
    struct sockaddr_storage remoteaddr; //client addr
    socklen_t addrlen;
    int newfd;

    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof(remoteaddr);

    newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);

    if(newfd == -1) {
    } else {
        if(set_socket_to_non_blocking(newfd) == -1) {
            cerr<<"Failed to set the socket to non blocking"<<endl;
            close(newfd);
            return;
        }
        add_to_pfds(pfds, newfd);
        conn_state_map[newfd] = {"", 0, string::npos, string::npos, "", 0,connection_state::READING};

        cout<<"pollserver: new connection from socket \n"<<inet_ntop2(&remoteaddr, remoteIP, sizeof(remoteIP))<<"\n"<<"the fd is: "<<newfd<<endl;
    }
}

void handle_recv(int sockfd, int newfd, vector<struct pollfd>& pfds, map<int, conn_state> &conn_state_map, int pfd_index) {
    cout<<"Handling a connection"<<endl;
    char temp_buf[MAXDATASIZE];

    conn_state &fd_state = conn_state_map[newfd];

    int numbytes = recv(newfd, temp_buf, sizeof(temp_buf) - 1, 0);
    if(numbytes == -1) {
        if(errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        } else {            
            del_from_pfds(pfds, pfd_index);
            conn_state_map.erase(newfd);
            close(newfd);
            return;
        }
    }
    //this means the request is done
    if(numbytes == 0) {
        fd_state.state = connection_state::SENDING;
        pfds[pfd_index].events = POLLOUT;
        return;
    };

    fd_state.recv_buf.append(temp_buf, numbytes);

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
            pfds[pfd_index].events = POLLOUT;
        }
    }
}

void handle_send(int sockfd, int newfd, vector<struct pollfd>& pfds, map<int, conn_state> &conn_state_map, int pfd_index) {
    conn_state &fd_state = conn_state_map[newfd];

    size_t remaining = fd_state.send_buf.size() - fd_state.send_offset;
    int sent = send(newfd, fd_state.send_buf.c_str() + fd_state.send_offset, remaining, 0);

    if(sent == -1) {
        if(errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        perror("send");
        close(newfd);
        conn_state_map.erase(newfd);
        del_from_pfds(pfds, pfd_index);
        return;
    }

    fd_state.send_offset += sent;
    
    if(fd_state.send_offset == fd_state.send_buf.size()) {
        close(newfd);
        conn_state_map.erase(newfd);
        del_from_pfds(pfds, pfd_index);
    }
}

int main() {
    //to handle clients disconnecting
    signal(SIGPIPE, SIG_IGN);

    int sockfd, newfd;
    sockfd = get_listener_socket();

    vector<struct pollfd> pfds;

    add_to_pfds(pfds, sockfd);
    
   
    cout<<"Server: waiting for connections"<<endl;

    while(1) {
        int ready_fds = poll(pfds.data(), pfds.size(), 1000);
        cout<<"ready_fds: "<<ready_fds<<endl;
        if(ready_fds == -1) {
            perror("poll");
            cout<<"Poll failed"<<endl;
            break;
        }
        if(ready_fds == 0) continue;
        for(int i = 0; i < pfds.size(); i++) {
            if(pfds[i].fd == sockfd && pfds[i].revents & POLLIN) {
                handle_new_connections(sockfd, pfds, conn_state_map);
                continue;

            }
            
            int old_size = pfds.size();
            if(pfds[i].revents & POLLIN) {
                handle_recv(sockfd, pfds[i].fd, pfds, conn_state_map, i);
            } else if(pfds[i].revents & POLLOUT) {
                handle_send(sockfd, pfds[i].fd, pfds, conn_state_map, i);
            }

            if(pfds.size() < old_size) i--;
        }
    }

    return 0;
}
