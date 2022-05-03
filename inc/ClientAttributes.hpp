#ifndef _CLIENT_ATTRIBUTES_HPP_
#define _CLIENT_ATTRIBUTES_HPP_

#include "Communication.hpp"
#include "ChatRoom.hpp"

#include <vector>
#include <string>
#include <list>

enum SessionState { MENU, CHATTING, DISCONNECTED };

class ClientAttributes
{
public:
    SOCKET communication_socket_fd;
    std::vector<ChatRoom> room_list;
    ChatRoom current_room;
    SessionState current_state;
    std::string nickname;
    std::string info_message;
    std::list<std::string> messages;
    int max_messages_on_screen = 10;

    void push_message(std::string message);
};

#endif