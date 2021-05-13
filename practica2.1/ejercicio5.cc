//g++ -o <nomEjec> <nom.cc>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <iostream>

const int MAX_BUFF_SIZE = 80;

int main(int argc, char **argv)
{
    struct addrinfo hints;
    struct addrinfo *res;

    memset((void *)&hints, 0, sizeof(addrinfo));

    if (argc < 3)
    {
        std::cerr << "Missing arguments:\n\tHere's an example: ./echo_client 0.0.0.0 3000\n";
        return -1;
    }

    // permite filtrar las direcciones
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

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

    struct sockaddr_in server;
    socklen_t serverlen = sizeof(sockaddr_in);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = INADDR_ANY;

    if (connect(sd, (const struct sockaddr *)&server, serverlen) == -1)
    {
        std::cerr << "Error connecting to the server " << errno << std::endl;
        close(sd);
        return -1;
    }

    while (true)
    {
        char* buff = NULL;
        size_t len = MAX_BUFF_SIZE;
        size_t line = getline(&buff, &len, stdin);

        if(buff[0] == 'Q' && (line == 1 || line == 2)){
            break;
        }

        if (send(sd, buff, strlen(buff), 0) == -1)
        {
            std::cerr << "Error sending bytes to server: " << errno << std::endl;
            close(sd);
            return -1;
        }

        int bytes = recv(sd, buff, strlen(buff), 0);

        if (bytes == -1)
        {
            std::cerr << "Error recieving data from server: " << errno << std::endl;
            close(sd);
            return -1;
        }

        std::cout << buff;
    }

    close(sd);
    return 0;
}