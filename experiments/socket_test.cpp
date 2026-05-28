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

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    cout<<"this is what s is: "<<s<<endl;

}

 

