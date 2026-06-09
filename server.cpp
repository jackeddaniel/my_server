#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#include "http.h"

#define PORT "3490"
#define BACKLOG 10
#define MAXDATASIZE 1000

using namespace std;

void sigchld_handler(int s) {
    (void)s;

    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void* get_in_addr(struct sockaddr* sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main() {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;

    socklen_t sin_size;

    struct sigaction sa;

    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    char recv_buf[MAXDATASIZE];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        cout<<gai_strerror(rv)<<endl;
        return 1;
    }

    //loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            cout<<"Socket building error"<<endl;
            perror("server: socket");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            cout<<"error"<<endl;
            perror("setsockopt");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            cout<<"Bind error"<<endl;
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL) {
        cout<<"server failed to bind"<<endl;
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1) {
        cout<<"Error while listening"<<endl;
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1) {
        cout<<"Sigaction"<<endl;
        perror("sigaction");
        exit(1);
    }
    
    cout<<"Server: waiting for connections"<<endl;

    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if(new_fd == -1) {
            cout<<"Accept error"<<endl;
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)& their_addr), s, sizeof(s));
        cout<<"Server got connection from: "<<s<<endl;

        int pid = fork();
        
        if(pid == -1) {
            perror("fork");
        } else if(pid == 0) {
            close(sockfd);
            string recv_str;
            char temp_buf[MAXDATASIZE];
            size_t header_end = string::npos;
            size_t content_length= 0;

            while(true) {
                int numbytes = recv(new_fd, temp_buf, sizeof(temp_buf)-1, 0);
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
                int sent = send(new_fd, response.c_str() + total_sent, response_len - total_sent, 0);
                if(sent == -1) {
                    perror("send");
                    break;
                }
                total_sent += sent;
            }

            close(new_fd);
            exit(0);

        } else {
            close(new_fd);
        }
    }

    return 0;
}
