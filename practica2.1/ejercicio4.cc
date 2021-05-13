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
        std::cerr << "Missing arguments:\n Here's an example: ./echo_server 0.0.0.0 3000\n";
        return -1;
    }

    // permite filtrar las direcciones
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(argv[1], argv[2], &hints, &res);

    if (ret != 0)
    {
        std::cerr << "Error: IP or port not known -> " << argv[1] << ":" << argv[2] << "\n";
        return -1;
    }

    int serverSd = socket(res->ai_family, res->ai_socktype, 0);

    if (serverSd == -1)
    {
        std::cerr << "Error creating the socket: " << errno << std::endl;
        freeaddrinfo(res);
        return -1;
    }

    int br = bind(serverSd, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);

    if (br != 0)
    {
        std::cerr << "Error: couldn't bind: " << errno << std::endl;
        close(serverSd);
        return -1;
    }

    if (listen(serverSd, 16) != 0)
    {
        std::cerr << "Error: couldn't listen: " << errno << std::endl;
        close(serverSd);
        return -1;
    }

    struct sockaddr client;
    socklen_t clientlen = sizeof(sockaddr);

    int clientSd = accept(serverSd, &client, &clientlen);

    if (clientSd == -1)
    {
        std::cerr << "Error: couldn't accept client: " << errno << std::endl;
        close(serverSd);
        return -1;
    }

    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    if (getnameinfo(&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
        std::cerr << "Error getting the name info from the client: " << errno << std::endl;
        close(serverSd);
        close(clientSd);
        return -1;
    }

    std::cout << "Conexion desde " << host << " " << serv << std::endl;

    while (true)
    {
        char buff[MAX_BUFF_SIZE] = {0};

        struct sockaddr client;
        socklen_t clientlen = sizeof(sockaddr);

        int bytes = recv(clientSd, buff, MAX_BUFF_SIZE, 0);

        if (bytes == -1)
        {
            std::cerr << "Error recieving data from user: " << errno << std::endl;
            close(serverSd);
            close(clientSd);
            return -1;
        }
        else if (bytes == 0)
        {
            std::cout << "Conexión terminada\n";
            break;
        }

        if (send(clientSd, buff, bytes, 0) == -1)
        {
            std::cerr << "Error sending bytes to client: " << errno << std::endl;
            close(serverSd);
            close(clientSd);
            return -1;
        }
    }

    close(serverSd);
    close(clientSd);
    return 0;
}