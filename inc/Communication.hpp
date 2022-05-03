#ifndef _COMMUNICATION_HPP_
#define _COMMUNICATION_HPP_

#include <string>

using SOCKET = int;

struct CommunicationData;

namespace Communication
{
    enum DataType { DISCONNECT, JOIN_ROOM, LEAVE_ROOM, CHAT_MESSAGE, ROOM_LIST, CREATE_ROOM, DELETE_ROOM, NONE };
    CommunicationData receive_data(SOCKET socket);

    void send_disconnect(SOCKET socket);
    void send_join_room(SOCKET socket, std::string room);
    void send_leave_room(SOCKET socket);
    void send_chat_message(SOCKET socket, std::string chat_message);
    void send_room_list(SOCKET socket, std::string room_list);
    void send_create_room(SOCKET socket, std::string room);
    void send_delete_room(SOCKET socket, std::string room);
}

struct CommunicationData
{
    Communication::DataType data_type;
    std::string data_content;
};

#endif