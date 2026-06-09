#include <iostream>
#include <string>
#include <poll.h>

using namespace std;

int main() {
    struct pollfd pfds[1];
    pfds[0].fd = 0;
    pfds[0].events = POLLIN;
    

    cout<<"HIT return or wait for 2.5s  of timeout"<<endl;

    int num_events = poll(pfds, 1, 2500);

    if(num_events == 0) {
        cout<<"poll timed out"<<endl;
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if(pollin_happened) {
            cout<<"File descriptor: "<<pfds[0].fd<<" is ready to read"<<endl;
        } else {
            cout<<"unexpected even occured"<<endl;
        }
    }

    return 0;
}
