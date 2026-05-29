#include <cstdint>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> 

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

    int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   
    connect(sock_fd, res->ai_addr, res->ai_addrlen);


    const char* msg = "Hello world";
    int len, bytes_sent;

    len = strlen(msg);

    bytes_sent = send(sock_fd, msg, len, 0);
}


