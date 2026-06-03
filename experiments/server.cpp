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

#define PORT "3490"
#define BACKLOG 10

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

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        cout<<"error"<<endl;
        return 1;
    }

    //loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            cout<<"Socket building error"<<endl;
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            cout<<"error"<<endl;
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            cout<<"Bind error"<<endl;
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
        exit(1);
    }

    sa.sa_handler = sigchld_handler;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1) {
        cout<<"Sigaction"<<endl;
        exit(1);
    }
    
    cout<<"Server: waitinf for connections"<<endl;

    while(1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if(new_fd == -1) {
            cout<<"Accept error"<<endl;
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)& their_addr), s, sizeof(s));
        cout<<"Server got connection from: "<<s<<endl;

        if(!fork()) {
            close(sockfd);
            if(send(new_fd, "Hello, world!", 13, 0) == -1) {
                cout<<"Send error"<<endl;
            }
            close(new_fd);
            exit(0);
        }

        close(new_fd);
    }

    return 0;
}
