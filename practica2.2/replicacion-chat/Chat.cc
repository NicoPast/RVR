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
    ChatMessage msgInp;
    ChatMessage msgOut;
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */
        Socket* client;
        
        int r = socket.recv(msgInp, client);

        if(r < 0){
            continue;
        }

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        switch (msgInp.type)
        {
            case ChatMessage::MessageType::LOGIN: {
                std::unique_ptr<Socket> soc(client);
                clients.push_back(std::move(soc));

                std::string m = msgInp.nick + " logged in.";
                //std::string m2 = "Connected users: " + std::to_string(clients.size());
                msgOut.type = ChatMessage::SERVER_MSG;
                msgOut.nick = "SERV";
                //msgOut.message = m2;
                //socket.send(msgOut, *client);
                msgOut.message = m;
                std::cout << m << "\nInfo: " << *client << "\n";
                break;
            }
            case ChatMessage::MessageType::LOGOUT: {
                auto it = clients.begin();
                bool found = false;
                while(it != clients.end() && !found){
                    if(*((*it).get()) == *client){
                        found = true;
                    }
                    else it++;
                }
                
                if(it != clients.end()){
                    std::string m = msgInp.nick + " logged out.";
                    clients.erase(it);
                    msgOut.type = ChatMessage::SERVER_MSG;
                    msgOut.nick = "Server";
                    msgOut.message = m;
                    std::cout << m << "\n";
                }
                else std::cout << "User not registered logged off\n";
                break;
            }
            case ChatMessage::MessageType::MESSAGE: {
                msgOut = msgInp;
                std::cout << msgInp.nick << " sent: " << msgInp.message << "\n";
                break;
            }
            default: 
                std::cout << "Message Type unknown " << msgInp.type << "\n";
                return;
        }

        //std::cout << "Connected users:\n";
        auto it = clients.begin();
        while(it != clients.end()) {
            // std::cout << *((*it).get()) << "\n";
            if(*((*it).get()) == *client){
                it++;
                continue;
            }
            socket.send(msgOut, *((*it).get()));
            it++;
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
    std::string m;

    ChatMessage msg(nick, m);
    msg.type = ChatMessage::LOGOUT;

    socket.send(msg, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        std::string inp;
        
        std::getline(std::cin, inp);

        // Enviar al servidor usando socket
        if(inp == "q"){
            ChatMessage msg(nick, inp);
            msg.type = ChatMessage::MessageType::LOGOUT;
            socket.send(msg, socket);
            return;
        }
        else{
            ChatMessage msg(nick, inp);
            msg.type = ChatMessage::MessageType::MESSAGE;
            socket.send(msg, socket);
        }
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        ChatMessage msgInp;
        Socket* s;

        int r = socket.recv(msgInp, s);

        if(r < 0){
            continue;
        }

        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        if(msgInp.type != ChatMessage::MessageType::SERVER_MSG){
            std::cout << msgInp.nick << ": " << msgInp.message << "\n";
        }
        else std::cout << msgInp.message << "\n";
    }
}