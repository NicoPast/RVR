//g++ -o <nomEjec> <nom.cc> -lpthread

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <iostream>

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
    messageThread(int sd, int id, int sleep = SLEEP_TIME) : _sd(sd), _shortId(id), _t(&messageThread::do_message, this), _sleepTime(sleep) {}
    ~messageThread()
    {
        std::cout << "Saliendo del thread: " << _id << " [" << _shortId << "]" << std::endl;
        _t.join();
    }
    void do_message()
    {
        _id = std::this_thread::get_id();
        while (active)
        {
            char buff[MAX_BUFF_SIZE] = {0};

            char host[NI_MAXHOST];
            char serv[NI_MAXSERV];

            struct sockaddr client;
            socklen_t clientlen = sizeof(sockaddr);

            int bytes = recvfrom(_sd, buff, MAX_BUFF_SIZE, 0, &client, &clientlen);

            if (bytes == -1)
            {
                if (errno != EAGAIN)
                {
                    std::cerr << "Thread " << _id << " [" << _shortId << "]: Error recieving data from user: " << errno << std::endl;
                    return;
                }
                else
                    continue;
            }

            if (getnameinfo(&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV) != 0)
            {
                std::cerr << "Thread " << _id << " [" << _shortId << "]: Error getting the name info from the client: " << errno << std::endl;
                return;
            }

            std::cout << "Thread " << _id << " [" << _shortId << "]: " << bytes << " bytes de " << host << ":" << serv << std::endl;

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
                if (SHUT_EVERYONE_OFF)
                {
                    std::cout << "Saliendo del thread: " << _id << " [" << _shortId << "]" << std::endl;
                    active = false;
                    return;
                }
            default:
                if (sendto(_sd, "Comando no soportado\n", 21, 0, &client, clientlen) == -1)
                {
                    std::cerr << "Thread " << _id << " [" << _shortId << "]: Error sending bytes to client: " << errno << std::endl;
                    return;
                }
                if (bytes < MAX_BUFF_SIZE - 1)
                {
                    buff[bytes] = '\n';
                    buff[bytes + 1] = '\0';
                }
                else
                {
                    buff[MAX_BUFF_SIZE - 2] = '\n';
                    buff[MAX_BUFF_SIZE - 1] = '\0';
                }
                std::cout << "Thread " << _id << " [" << _shortId << "]: Comando no soportado " << buff;
            }

            if (flag != "")
            {
                char val[MAX_BUFF_SIZE];
                int tam = strftime(val, MAX_BUFF_SIZE, flag, t);
                if (tam < MAX_BUFF_SIZE - 1)
                {
                    val[tam] = '\n';
                    val[tam + 1] = '\0';
                }
                else
                {
                    val[MAX_BUFF_SIZE - 2] = '\n';
                    val[MAX_BUFF_SIZE - 1] = '\0';
                }

                if (sendto(_sd, val, tam + 1, 0, &client, clientlen) == -1)
                {
                    std::cerr << "Thread " << _id << " [" << _shortId << "]: Error sending bytes to client: " << errno << std::endl;
                    return;
                }
            }

            std::cout << "Thread " << _id << " [" << _shortId << "] will sleep for " << _sleepTime << "s\n";
            for (int i = 0; i < _sleepTime; i++)
            {
                sleep(1);
            }

            std::cout << "Thread " << _id << " [" << _shortId << "] is awake!\n";
        }
    }

private:
    int _sd;
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

int main(int argc, char **argv)
{
    struct addrinfo hints;
    struct addrinfo *res;

    memset((void *)&hints, 0, sizeof(addrinfo));

    if (argc < 3)
    {
        std::cerr << "Missing arguments:\n Here's an example: ./time_server_th 0.0.0.0 3000\n";
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

    int sd = socket(res->ai_family, res->ai_socktype | SOCK_NONBLOCK, 0);

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

    messageThread *msgThs[MAX_THREAD];

    for (int i = 0; i < MAX_THREAD; i++)
    {
        msgThs[i] = new messageThread(sd, i);
    }

    auto future = std::async(std::launch::async, GetLineFromCin);
    while (active)
    {
        if (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready)
        {
            auto line = future.get();
            future = std::async(std::launch::async, GetLineFromCin);
            std::cout << line << std::endl;

            if (line == "q")
            {
                active = false;
            }
        }
    }

    std::cout << "Shuting server off...\n";

    for (int i = 0; i < MAX_THREAD; i++)
    {
        delete msgThs[i];
    }

    close(sd);
    std::cout << "Press enter to close...\n";
    return 0;
}