#ifndef _USER_INPUT_HANDLING_HPP_
#define _USER_INPUT_HANDLING_HPP_

#include <vector>
#include <string>

#include "Communication.hpp"
#include "ChatRoom.hpp"
#include "ClientAttributes.hpp"

class UserInput
{
public:
    enum InputType { DISCONNECT, JOIN_ROOM, LEAVE_ROOM, CREATE_ROOM, DELETE_ROOM, SEND_MESSAGE, INVALID };

    UserInput();
    UserInput(std::string cmd, ClientAttributes & client_attributes);

    std::string raw_input;
    std::vector<std::string> parsed_input;
    InputType input_type;
    std::string details;

    void parse_input();
    bool str_is_number(const std::string& s);

    void validate_input(ClientAttributes & client_attributes);
    bool join_room_is_valid(ClientAttributes client_attributes);
    bool leave_room_is_valid(ClientAttributes client_attributes);
    bool create_room_is_valid(ClientAttributes client_attributes);
    bool delete_room_is_valid(ClientAttributes client_attributes);
    bool send_message_is_valid(ClientAttributes client_attributes);
};


#endif