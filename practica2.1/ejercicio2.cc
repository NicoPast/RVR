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

    if (argc < 3)
    {
        std::cerr << "Missing arguments:\n Here's an example: ./time_server 0.0.0.0 3000\n";
        return -1;
    }

    // permite filtrar las direcciones
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

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

    int br = bind(sd, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);

    if (br != 0)
    {
        std::cerr << "Error: couldn't bind: " << errno << std::endl;
        close(sd);
        return -1;
    }

    while (true)
    {
        char buff[MAX_BUFF_SIZE] = {};

        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];

        struct sockaddr client;
        socklen_t clientlen = sizeof(sockaddr);

        int bytes = recvfrom(sd, buff, MAX_BUFF_SIZE, 0, &client, &clientlen);

        if (bytes == -1)
        {
            std::cerr << "Error recieving data from user: " << errno << std::endl;
            close(sd);
            return -1;
        }

        if (getnameinfo(&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
        {
            std::cerr << "Error getting the name info from the client: " << errno << std::endl;
            close(sd);
            return -1;
        }

        std::cout << bytes << " bytes de " << host << ":" << serv << std::endl;

        struct tm *t;
        time_t timeVar;

        time(&timeVar);
        t = localtime(&timeVar);

        const char *flag = "";
        switch (buff[0])
        {
        case 't':
            flag = "%r";
            break;
        case 'd':
            flag = "%F";
            break;
        case 'q':
            std::cout << "Saliendo...\n";
            close(sd);
            return 0;
        default:
            if (sendto(sd, "Comando no soportado\n", 21, 0, &client, clientlen) == -1)
            {
                std::cerr << "Error sending bytes to client: " << errno << std::endl;
                close(sd);
                return -1;
            }
            buff[bytes] = '\n';
            std::cout << "Comando no soportado " << buff;
            continue;
        }

        if (flag != "")
        {
            char val[MAX_BUFF_SIZE];
            int tam = strftime(val, MAX_BUFF_SIZE, flag, t);
            val[tam] = '\n';

            if (sendto(sd, val, tam+1, 0, &client, clientlen) == -1)
            {
                std::cerr << "Error sending bytes to client: " << errno << std::endl;
                close(sd);
                return -1;
            }
        }
    }

    close(sd);
    return 0;
}