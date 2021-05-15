//g++ -o <nomEjec> <nom.cc> -lpthread

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <iostream>
#include <list>

#include <thread>

#include <time.h>
#include <chrono>
#include <future>

const int MAX_BUFF_SIZE = 80;
const int MAX_THREAD = 5;
const int SLEEP_TIME = 3;
const bool SHUT_EVERYONE_OFF = false;

bool active = true;

class messageThread
{
public:
    messageThread() {}
    messageThread(int sd, int id, int sleep = SLEEP_TIME) : _clientSd(sd), _shortId(id), _t(&messageThread::do_conexion, this), _sleepTime(sleep) 
    {
        _t.detach();
    }
    ~messageThread()
    {
        std::cout << "Saliendo del thread: " << _id << " [" << _shortId << "]" << std::endl;
        _t.join();
    }
    void do_conexion()
    {
        _id = std::this_thread::get_id();
        while (true)
        {
            char buff[MAX_BUFF_SIZE] = {0};

            int bytes = recv(_clientSd, buff, MAX_BUFF_SIZE, 0);

            if (bytes == -1)
            {
                std::cerr << "Thread " << _id  << " [" << _shortId << "]: Error recieving data from user: " << errno << std::endl;
                close(_clientSd);
                return;
            }
            else if (bytes == 0)
            {
                std::cout << "Thread " << _id  << " [" << _shortId << "]: Conexión terminada\n";
                break;
            }

            if (send(_clientSd, buff, bytes, 0) == -1)
            {
                std::cerr << "Thread " << _id  << " [" << _shortId << "]: Error sending bytes to client: " << errno << std::endl;
                close(_clientSd);
                return;
            }
        }
        close(_clientSd);
    }

private:
    int _clientSd;
    std::thread _t;
    std::thread::id _id;
    int _shortId;
    int _sleepTime;
};

std::string GetLineFromCin()
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void do_conexion(int clientSd, int id)
    {
        while (true)
        {
            char buff[MAX_BUFF_SIZE] = {0};

            int bytes = recv(clientSd, buff, MAX_BUFF_SIZE, 0);

            if (bytes == -1)
            {
                std::cerr << "[" << id << "] Error recieving data from user: " << errno << std::endl;
                close(clientSd);
                return;
            }
            else if (bytes == 0)
            {
                std::cout << "[" << id << "] Conexión terminada\n";
                break;
            }

            if (send(clientSd, buff, bytes, 0) == -1)
            {
                std::cerr << "[" << id << "] Error sending bytes to client: " << errno << std::endl;
                close(clientSd);
                return;
            }
        }
        close(clientSd);
    }

int main(int argc, char **argv)
{
    struct addrinfo hints;
    struct addrinfo *res;

    memset((void *)&hints, 0, sizeof(addrinfo));

    if (argc < 3)
    {
        std::cerr << "Missing arguments:\n Here's an example: ./echo_server_th 0.0.0.0 3000\n";
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

    int id = 0;

    std::list<messageThread*> mThList;

    while (true)
    {
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

        id++;

        mThList.push_front(new messageThread(clientSd, id));
    }

    close(serverSd);
    return 0;
}