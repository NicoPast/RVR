#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data

    char* b = _data;

    memcpy(b, &type, sizeof(type));
    b += sizeof(type);

    memcpy(b, nick.c_str(), sizeof(char) * NICK_SIZE);
    b += sizeof(char) * NICK_SIZE;

    memcpy(b, message.c_str(), sizeof(char) * message.length());
    b += sizeof(char) * MSG_SIZE;
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data

    char* b = _data;

    memcpy(&type, b, sizeof(type));
    b += sizeof(type);

    nick = std::string(sizeof(char) * NICK_SIZE, '\0');
    memcpy((void *)nick.c_str(), b, sizeof(char) * NICK_SIZE);
    b += sizeof(char) * NICK_SIZE;

    message = std::string(sizeof(char) * MSG_SIZE, '\0');
    memcpy((void *)message.c_str(), b, sizeof(char) * MSG_SIZE);

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */
        Socket* s;
        ChatMessage msg;
        socket.recv(msg, s);
        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        switch (msg.type)
        {
        case ChatMessage::MessageType::LOGIN:
            /* code */
            break;
        case ChatMessage::MessageType::LOGOUT:
            /* code */
            break;
        case ChatMessage::MessageType::MESSAGE:
            /* code */
            break;
        default:
            std::cout << "Message Type unknown " << msg.type << "\n";
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}