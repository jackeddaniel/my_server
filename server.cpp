#include <asm-generic/socket.h>
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

#include <vector>
#include <string>

#include "http.h"

#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 1000

using namespace std;

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

void handle_new_connections(int listener, vector<struct pollfd> &pfds) {
    cout<<"Handling a new connection"<<endl;
    struct sockaddr_storage remoteaddr; //client addr
    socklen_t addrlen;
    int newfd;

    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof(remoteaddr);

    newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);

    if(newfd == -1) {
    } else {
        add_to_pfds(pfds, newfd);

        cout<<"pollserver: new connection from socket \n"<<inet_ntop2(&remoteaddr, remoteIP, sizeof(remoteIP))<<"\n"<<"the fd is: "<<newfd<<endl;
    }
}

void handle_connection(int sockfd, int newfd, vector<struct pollfd>& pfds) {
    cout<<"Handling a connection"<<endl;
    //close(sockfd);
    string recv_str;
    char temp_buf[MAXDATASIZE];
    size_t header_end = string::npos;
    size_t content_length= 0;

    while(true) {
        int numbytes = recv(newfd, temp_buf, sizeof(temp_buf)-1, 0);
        if(numbytes == -1) {perror("recv"); exit(1);}
        if(numbytes == 0) break;

        recv_str.append(temp_buf, numbytes);

        if(header_end == string::npos) {
            header_end = recv_str.find("\r\n\r\n");

            if(header_end != string::npos) {
                size_t cl_pos = recv_str.find("Content-Length");
                
                if(cl_pos != string::npos) {
                    size_t val_start = recv_str.find(": ", cl_pos) + 2;
                    size_t val_end = recv_str.find("\r\n", val_start);
                    content_length = stoi(recv_str.substr(val_start, val_end - val_start));
                }
            }
        }

        if(header_end != string::npos) {
            size_t total_expected = header_end + 4 + content_length; // the 4 is to accomodate for the chards \r\n\r\n
            if(recv_str.size() >= total_expected) break;
        }
    }

    http_request request = build_request(recv_str);
    http_response resp;
    string path;

    if(path_builder(request.path, path) == 0) {
        resp= create_response(404, "", "text/html");
    } else {
        string body = html_to_string(path);
        resp = create_response(200, body, infer_content_type(path));
    }

    string response = build_response(resp);
    cout<<"The message recieved from the browser"<<endl;
    cout<<"The size of the message recieved is: "<<recv_str.size()<<" bytes"<<endl;
    cout<<recv_str<<endl;
    cout<<"=========================="<<endl;
    cout<<endl;
    cout<<"Sending the response"<<endl;

    size_t total_sent = 0;
    size_t response_len = response.size();
    while(total_sent < response_len) {
        int sent = send(newfd, response.c_str() + total_sent, response_len - total_sent, 0);
        if(sent == -1) {
            perror("send");
            break;
        }
        total_sent += sent;
    }

    close(newfd);
    int index = -1;

    for(int i = 0; i < pfds.size(); i++ ) {
        if(pfds[i].fd == newfd) {
            index = i;
            break;
        }
    }

    del_from_pfds(pfds, index);
    //exit(0);
}

int main() {
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
                handle_new_connections(sockfd, pfds);
                continue;

            }

            if(pfds[i].revents & POLLIN) {
                handle_connection(sockfd, pfds[i].fd, pfds);
            }
        }
    }

    return 0;
}
