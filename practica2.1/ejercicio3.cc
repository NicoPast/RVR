//g++ -o <nomEjec> <nom.cc>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <iostream>
#include <iomanip>

#include <time.h>

const int MAX_BUFF_SIZE = 80;

int main(int argc, char **argv)
{
    struct addrinfo hints;
    struct addrinfo *res;

    memset((void *)&hints, 0, sizeof(addrinfo));

    if (argc < 4)
    {
        std::cerr << "Missing arguments:\n\tHere's an example: ./time_client 0.0.0.0 3000 t\n";
        return -1;
    }

    // permite filtrar las direcciones
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int ret = getaddrinfo(argv[1], argv[2], &hints, &res);

    if (ret != 0)
    {
        std::cerr << "Error: IP or port not known -> " << argv[1] << ":" << argv[2] << "\n";
        return -1;
    }

    int sd = socket(res->ai_family, res->ai_socktype, 0);

    if (sd == -1)
    {
        std::cerr << "Error creating the socket: " << errno << std::endl;
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);

    char buff[MAX_BUFF_SIZE];

    struct sockaddr_in server;
    socklen_t serverlen = sizeof(sockaddr_in);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = INADDR_ANY;

    if (sendto(sd, argv[3], strlen(argv[3]), 0, (const struct sockaddr *)&server, serverlen) == -1)
    {
        std::cerr << "Error sending bytes to server: " << errno << std::endl;
        close(sd);
        return -1;
    }
    if (argv[3][0] != 'q')
    {
        int bytes = recvfrom(sd, buff, MAX_BUFF_SIZE, 0, (struct sockaddr *)&server, &serverlen);

        if (bytes == -1)
        {
            std::cerr << "Error recieving data from server: " << errno << std::endl;
            close(sd);
            return -1;
        }

        buff[bytes] = '\n';

        std::cout << buff;
    }

    close(sd);
    return 0;
}