#include "UserInputHandling.hpp"

#include <iostream>

UserInput::UserInput(std::string cmd, ClientAttributes & client_attributes)
{
    raw_input = cmd;
    parse_input();
    validate_input(client_attributes);

    switch (input_type)
    {
    case DISCONNECT:
        // if (client_attributes.current_state == CHATTING)
        //     Communication::send_chat_message(client_attributes.communication_socket_fd, "/ left the room /");
        Communication::send_disconnect(client_attributes.communication_socket_fd);
        client_attributes.current_state = DISCONNECTED;
        break;
    case JOIN_ROOM:
        client_attributes.current_room = client_attributes.room_list.at(std::stoi(parsed_input[1]));
        Communication::send_join_room(client_attributes.communication_socket_fd, parsed_input[1]);
        client_attributes.current_state = CHATTING;
        // Communication::send_chat_message(client_attributes.communication_socket_fd, "/ joined the room /");
        break;
    case LEAVE_ROOM:
        // Communication::send_chat_message(client_attributes.communication_socket_fd, "/ left the room /");
        Communication::send_leave_room(client_attributes.communication_socket_fd);
        client_attributes.current_room = ChatRoom();
        client_attributes.current_state = MENU;
        client_attributes.messages.clear();
        break;
    case CREATE_ROOM:
        client_attributes.current_room = ChatRoom(parsed_input[1], std::stoi(parsed_input[2]), 0);
        Communication::send_create_room(client_attributes.communication_socket_fd, client_attributes.current_room.to_string());
        break;
    case DELETE_ROOM:
        Communication::send_delete_room(client_attributes.current_state, client_attributes.current_room.to_string());
        client_attributes.current_state = MENU;
        break;
    case SEND_MESSAGE:
        Communication::send_chat_message(client_attributes.communication_socket_fd, raw_input);
        break;
    default:
        break;
    }
}

void UserInput::parse_input()
{
    std::string cmd = raw_input;
    while (cmd.back() == ' ' || cmd.back() == '\t') cmd.pop_back();
    while (cmd.front() == ' ' || cmd.front() == '\t') cmd.erase(0, 1);

    if (!cmd.empty() && cmd[0] != '/') {
        parsed_input.push_back(cmd);
        return;
    }
    cmd.push_back(' ');

    std::string cur_word;
    for (int i = 0; i < (int) cmd.size(); i++) {
        if (cur_word.empty() && (cmd[i] == ' ' || cmd[i] == '\t')) {
            continue;
        }
        else if (!cur_word.empty() && (cmd[i] == ' ' || cmd[i] == '\t')) {
            parsed_input.push_back(cur_word);
            cur_word.clear();
        }
        else if (cmd[i] != ' ' && cmd[i] != '\t'){
            cur_word.push_back(cmd[i]);
        }
    }
}

bool UserInput::str_is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void UserInput::validate_input(ClientAttributes & client_attributes)
{
    if (parsed_input.empty()) {
        details = "Command cannot be blank";
        input_type = INVALID;
    }
    else if (parsed_input[0] == "/disconnect")
        input_type = DISCONNECT;
    else if (parsed_input[0] == "/join")
        input_type = join_room_is_valid(client_attributes) ? JOIN_ROOM : INVALID;
    else if (parsed_input[0] == "/leave")
        input_type = leave_room_is_valid(client_attributes) ? LEAVE_ROOM : INVALID;
    else if (parsed_input[0] == "/create")
        input_type = create_room_is_valid(client_attributes) ? CREATE_ROOM : INVALID;
    else if (parsed_input[0] == "/delete")
        input_type = delete_room_is_valid(client_attributes) ? DELETE_ROOM : INVALID;
    else if (parsed_input[0][0] == '/') {
        details = "Command " + parsed_input[0] + " does not exist";
        input_type = INVALID;
    }
    else {
        input_type = send_message_is_valid(client_attributes) ? SEND_MESSAGE : INVALID;
    }
}

bool UserInput::join_room_is_valid(ClientAttributes client_attributes)
{
    if (client_attributes.current_state == CHATTING) {
        details = "'join' command is not allowed when inside a room. You can leave this room by using '/leave' command";
        return false;
    }
    if ((int) parsed_input.size() != 2) {
        details = "Wrong usage of command 'join'. Usage: /join <room_identifier>";
        return false;
    }
    if (!str_is_number(parsed_input[1])) {
        details = "Room identifier must be integer";
        return false;
    }
    int room_identifier = std::stoi(parsed_input[1]);
    if (room_identifier < 0 || room_identifier >= (int) client_attributes.room_list.size()) {
        details = "There is no room with identifier '" + parsed_input[1] + "'";
        return false;
    }
    if (client_attributes.room_list.at(room_identifier).current_participants == client_attributes.room_list.at(room_identifier).max_participants) {
        details = "Room '" + parsed_input[1] + "' is full";
        return false;
    }
    return true;
}

bool UserInput::leave_room_is_valid(ClientAttributes client_attributes)
{
    if (client_attributes.current_state != CHATTING) {
        details = "'leave' should only be used when inside a room";
        return false;
    }
    if ((int) parsed_input.size() != 1) {
        details = "Wrong usage of command 'leave'. Usage: /leave";
        return false;
    }
    return true;
}

bool UserInput::create_room_is_valid(ClientAttributes client_attributes)
{
    if (client_attributes.current_state != MENU) {
        details = "'create' command is only allowed on the menu screen.";
        return false;
    }
    if ((int)parsed_input.size() != 3) {
        details = "Wrong usage of command 'create'. Usage: /create <room_name> <max_participants>";
        return false;
    }
    if ((int)parsed_input[1].size() > 15 || (int)parsed_input[1].size() < 3) {
        details = "Room name must be at least and at most 15 characters long.";
        return false;
    }
    if (ChatRoom::name_taken(parsed_input[1])) {
        details = "There is already a room with this name: " + parsed_input[1];
        return false;
    }
    if (!str_is_number(parsed_input[2])) {
        details = "Max participants must be an integer";
        return false;
    }
    int max_participants = std::stoi(parsed_input[2]);
    if (max_participants > 100 || max_participants < 1) {
        details = "A room can only admit at least 1 and at most 100 participants";
        return false;
    }
    return true;
}

bool UserInput::delete_room_is_valid(ClientAttributes client_attributes)
{
    details = "Blocked functionality";
    client_attributes.info_message = details;
    return false;
}

bool UserInput::send_message_is_valid(ClientAttributes client_attributes)
{
    if (CHATTING != client_attributes.current_state) {
        details = "Invalid command";
        return false;
    }
    if (255 < (int) raw_input.size()) {
        details = "Your message can be at most 255 characters long";
        return false;
    }
    return true;
}