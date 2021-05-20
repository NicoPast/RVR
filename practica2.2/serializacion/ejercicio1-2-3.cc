#include "Serializable.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

class Jugador: public Serializable
{
private:
    static const size_t MAX_NAME = 20;

    char name[MAX_NAME];

    int16_t x;
    int16_t y;

    int32_t sData;

public:
    Jugador(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y)
    {
        strncpy(name, _n, MAX_NAME);
        name[MAX_NAME-1] = '\0';
        sData = sizeof(x) + sizeof(y) + sizeof(char) * MAX_NAME;
    };

    virtual ~Jugador(){};

    void to_bin()
    {
        //
        alloc_data(sData);

        memset(_data, 0, sData);

        char* b = data();

        //memcpy(b, &sData, sizeof(sData));
        memcpy(b, &x, sizeof(x));
        memcpy(b + sizeof(x), &y, sizeof(y));
        memcpy(b + sizeof(x) + sizeof(y), &name, sizeof(char) * MAX_NAME);
    }

    int from_bin(char * data)
    {
        //
        alloc_data(sData);

        char* b = data;

        memcpy(&x, b, sizeof(x));
        memcpy(&y, b + sizeof(x), sizeof(y));
        memcpy(name, b + sizeof(x) + sizeof(y), sizeof(char) * MAX_NAME);

        return 0;
    }

    int getX(){
        return x;
    }

    int getY(){
        return y;
    }

    char* getName(){
        return name;
    }

    void print(){
        std::cout << "Name: " << name << "\nX: " << x << "\nY: " << y << "\n";
    }
};

int main(int argc, char **argv)
{
    Jugador one_r("", 0, 0);
    Jugador one_w("Player_ONE", 123, 456);

    // Ejercicio 2

    // 1. Serializar el objeto one_w
    one_w.to_bin();

    // 2. Escribir la serialización en un fichero
    int fd = open("./data.jugador", O_CREAT | O_TRUNC | O_RDWR, 0666);

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
    fd = open("./data.jugador", O_CREAT | O_RDWR, 0666);

    if(fd == -1){
        std::cout << "Error opening the file\n";
        return -1;
    }

    char* buff = (char*)malloc(one_r.size());

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
    one_r.print();
    
    return 0;
}

