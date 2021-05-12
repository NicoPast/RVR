//g++ -o <nomEjec> <nom.cc>

#include <sys/socket.h> 
#include <netdb.h>
#include <sys/types.h>

#include <string.h>
#include <iostream>
#include <iomanip>

int main(int argc, char **argv){
    struct addrinfo hints;
    struct addrinfo* res;

    memset((void *) &hints, 0, sizeof(addrinfo));

    if(argc < 2) {
        std::cerr << "Missing arguments:\n Here's an example: ./gai www.google.com http\n";
        return -1;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(argv[1], argv[2], NULL, &res);

    if(ret != 0) {
        std::cerr << "Error: Name or service not known -> " << argv[1] << "\n";
        return -1;
    }

    for(auto i = res; i != nullptr; i = i->ai_next) {
        char host[NI_MAXHOST];
        getnameinfo(i->ai_addr, i->ai_addrlen, host, NI_MAXHOST, NULL, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        std::cout << std::left << "Host: " << std::setw(25) << host 
                  << " Family: " << std::setw(3) << i->ai_family 
                  << " SocketType: " << i->ai_socktype << std::endl;
    }

    return 0;
}