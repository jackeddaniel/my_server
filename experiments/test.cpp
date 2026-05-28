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
    struct addrinfo* servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // Don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // Fill in my IP for me (listening socket)

    // FIX: Added parentheses around the assignment so status gets the return value
    if ((status = getaddrinfo("google.com", "3490", &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo error: " << gai_strerror(status) << endl;
        return 1;
    }

    // --- PRINTING SERVINFO ---
    cout << "getaddrinfo successful! Iterating through results:\n\n";
    
    struct addrinfo* p = servinfo;
    int count = 1;

    while (p != nullptr) {
        cout << "--- Result #" << count++ << " ---" << endl;
        
        // 1. Print the IP Family
        cout << "Family: ";
        if (p->ai_family == AF_INET) cout << "IPv4 (AF_INET)" << endl;
        else if (p->ai_family == AF_INET6) cout << "IPv6 (AF_INET6)" << endl;
        else cout << "Unknown (" << p->ai_family << ")" << endl;

        // 2. Print Socket Type
        cout << "Socket Type: ";
        if (p->ai_socktype == SOCK_STREAM) cout << "TCP (SOCK_STREAM)" << endl;
        else if (p->ai_socktype == SOCK_DGRAM) cout << "UDP (SOCK_DGRAM)" << endl;

        // 3. Extract and print the actual IP Address and Port
        char ipstr[INET6_ADDRSTRLEN];
        void* addr;
        uint16_t port;

        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            port = ntohs(ipv4->sin_port);
        } else { // IPv6
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            port = ntohs(ipv6->sin6_port);
        }

        // Convert the IP byte network data to a human-readable string
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        cout << "IP Address: " << ipstr << endl;
        cout << "Port:       " << port << endl;
        cout << endl;

        p = p->ai_next; // Move to the next struct in the linked list
    }

    freeaddrinfo(servinfo); // Clean up memory
    return 0;
}
