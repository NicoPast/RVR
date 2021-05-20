#include <thread>
#include "Chat.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// int main(int argc, char **argv)
// {
//     ChatClient ec(argv[1], argv[2], argv[3]);

//     std::thread net_thread([&ec](){ ec.net_thread(); });

//     ec.login();

//     ec.input_thread();
// }
int main(int argc, char **argv)
{
    ChatMessage one_r("Aquiles","Las Termopilas han caido");
    ChatMessage one_w("Aquiles","Las Termopilas han caido");
    one_w.type = 0;

    // Ejercicio 2

    // 1. Serializar el objeto one_w
    one_w.to_bin();

    // 2. Escribir la serialización en un fichero
    int fd = open("./data.msg", O_CREAT | O_TRUNC | O_RDWR, 0666);

    if(fd == -1){
        std::cout << "Error opening the file\n";
        return -1;
    }
    
    int b = write(fd, one_w.data(), one_w.size());
    if(b == -1){
        std::cout << "Error writting from file\n";
        close(fd);
        return -1;
    }
    close(fd);

    // Ejercicio 3

    // 3. Leer el fichero 
    fd = open("./data.msg", O_CREAT | O_RDWR, 0666);

    if(fd == -1){
        std::cout << "Error opening the file\n";
        return -1;
    }

    char* buff = (char*)malloc(one_w.size());

    b = read(fd, buff, one_w.size());

    if(b == -1){
        std::cout << "Error reading from file\n";
        close(fd);
        return -1;
    }
    close(fd);

    // 4. "Deserializar" en one_r
    one_r.from_bin(buff);

    // 5. Mostrar el contenido de one_r
    std::cout << "Nick: " << one_w.nick << "\nMessage: " << one_w.message << "\n";

    std::cout << "Nick: " << one_r.nick << "\nMessage: " << one_r.message << "\n";
    
    return 0;
}