#include "Communication.hpp"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

static char buffer[4096];

CommunicationData Communication::receive_data(SOCKET socket)
{
    CommunicationData data_received;

    memset(buffer, 0, sizeof(buffer));
    recv(socket, buffer, sizeof(buffer), 0);
    switch (buffer[0]) {
        case 'D': data_received.data_type = Communication::DISCONNECT; break;
        case 'J': data_received.data_type = Communication::JOIN_ROOM; break;
        case 'L': data_received.data_type = Communication::LEAVE_ROOM; break;
        case 'C': data_received.data_type = Communication::CHAT_MESSAGE; break;
        case 'R': data_received.data_type = Communication::ROOM_LIST; break;
        case 'E': data_received.data_type = Communication::CREATE_ROOM; break;
        case 'T': data_received.data_type = Communication::DELETE_ROOM; break;
        default: data_received.data_type = Communication::DISCONNECT; break;
    }

    data_received.data_content = buffer + 1;
    return data_received;
}

void Communication::send_disconnect(SOCKET socket)
{
#ifndef TESTS
    std::string content = "D";
    send(socket, content.c_str(), content.size(), 0);
#endif
}

void Communication::send_join_room(SOCKET socket, std::string room)
{
#ifndef TESTS
    std::string content = "J" + room;
    send(socket, content.c_str(), content.size(), 0);
#endif
}

void Communication::send_leave_room(SOCKET socket)
{
#ifndef TESTS
    std::string content = "L";
    send(socket, content.c_str(), content.size(), 0);
#endif
}

void Communication::send_chat_message(SOCKET socket, std::string chat_message)
{
#ifndef TESTS
    std::string content = "C" + chat_message;
    send(socket, content.c_str(), content.size(), 0);
#endif
}

void Communication::send_room_list(SOCKET socket, std::string room_list)
{
#ifndef TESTS
    std::string content = "R" + room_list;
    send(socket, content.c_str(), content.size(), 0);
#endif
}

void Communication::send_create_room(SOCKET socket, std::string room)
{
#ifndef TESTS
    std::string content = "E" + room;
    send(socket, content.c_str(), content.size(), 0);
#endif
}

void Communication::send_delete_room(SOCKET socket, std::string room)
{
#ifndef TESTS
    std::string content = "T" + room;
    send(socket, content.c_str(), content.size(), 0);
#endif
}