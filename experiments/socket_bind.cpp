#include <cstdint>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> // Added for inet_ntop to print IP addresses

using namespace std;

int main() {
    int status;
    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // Fill in my IP for me (listening socket)

    if ((status = getaddrinfo(nullptr, "3490", &hints, &res)) != 0) {
        cerr << "getaddrinfo error: " << gai_strerror(status) << endl;
        return 1;
    }

    //we make a socket
    int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    cout<<"this is what s is: "<<sock_fd<<endl;

    int bind_status = bind(sock_fd, res->ai_addr, res->ai_addrlen);

    if(bind_status == -1) {
        cout<<"Bind failed"<<endl;
    }

    //let's listen in this port
    listen(sock_fd, 3);

    struct sockaddr_storage their_addr;
    socklen_t addr_size;

    addr_size = sizeof(their_addr);
    int new_fd = accept(sock_fd, (struct sockaddr*)&their_addr, &addr_size);

    cout<<"The fd: "<<new_fd<<endl;

    char msg[100];
    memset(msg, 0, 100);

    int bytes_recieved = recv(new_fd, msg, 100, 0);   

    msg[bytes_recieved] = '\0';

    cout<<msg<<endl;
}


