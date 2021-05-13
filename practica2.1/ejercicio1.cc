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
        std::cerr << "Missing arguments:\n Here's an example: ./gai www.google.com || ./gai www.google.com http\n";
        return -1;
    }

    // permite filtrar las direcciones
    hints.ai_family = AF_UNSPEC;

    int ret = getaddrinfo(argv[1], argv[2], &hints, &res);

    if(ret != 0) {
        std::cerr << "Error: Name or service not known -> " << argv[1] << " " << argv[2] << "\n";
        return -1;
    }

    for(auto i = res; i != nullptr; i = i->ai_next) {
        char host[NI_MAXHOST];
        
        if(getnameinfo(i->ai_addr, i->ai_addrlen, host, NI_MAXHOST, NULL, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0){
            std::cerr << "Error getting the name info from one of the dirs\n";
            freeaddrinfo(res);
            return -1;
        }

        std::cout << std::left << "Host: " << std::setw(25) << host 
                  << " Family: " << std::setw(3) << i->ai_family 
                  << " SocketType: " << i->ai_socktype << std::endl;
    }

    freeaddrinfo(res);
    
    return 0;
}