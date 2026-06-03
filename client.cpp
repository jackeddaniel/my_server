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

#define MAXDATASIZE 100

using namespace std;

void* get_in_addr(struct sockaddr* sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);

    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if(argc != 2) {
        cout<<"usage hostname"<<endl;
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        cout<<"getaddrinfo err"<<endl;
        return 1;

    }

    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            cout<<"client socket err"<<endl;
            continue;
        }


        inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof(s));
        cout<<"client attempting connection: "<<s<<endl;

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
            cout<<"client connect error"<<endl;
            close(sockfd);
            continue;
        }
        break;
    }

    if(p == nullptr) {
        cout<<"clien failed to connect"<<endl;
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof(s));
    cout<<"client connected to: "<<s<<endl;

    freeaddrinfo(servinfo);

    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        cout<<"recv error"<<endl;
        exit(1);
    }

    buf[numbytes] = '\0';
    cout<<buf<<endl;
    close(sockfd);
    return 0;

}
