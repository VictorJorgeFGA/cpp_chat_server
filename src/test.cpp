#define TESTS

#include "ChatRoom.hpp"
#include "UserInputHandling.hpp"

#include <iostream>
#include <cassert>

void chat_room_tests()
{
    std::cout << "CHAT ROOM TESTS:" << std::endl;
    std::vector<ChatRoom> chat_room_list;
    chat_room_list.push_back(ChatRoom("room 1", 10, 5));
    chat_room_list.push_back(ChatRoom("room 2", 5, 5));
    chat_room_list.push_back(ChatRoom("room 3", 5, 3));

    std::string expected_string = "room 1;10;5;room 2;5;5;room 3;5;3;";
    assert(expected_string == ChatRoom::serialize_room_list(chat_room_list));

    auto new_chat_room_list = ChatRoom::deserialize_room_list(expected_string);
    assert(chat_room_list.size() == new_chat_room_list.size());

    for (int i = 0; i < (int) chat_room_list.size(); i++)
        assert(chat_room_list[i] == new_chat_room_list[i]);

    auto room_1 = chat_room_list.front();
    assert ("room 1;10;5;" == room_1.to_string());
    assert (ChatRoom("room 1;10;5;") == room_1);

    std::cout << "Ok" << std::endl << std::endl;
}

void user_input_handling_tests()
{
    std::cout << "USER INPUT HANDLING TESTS:" << std::endl;
    ClientAttributes ca;

    std::string cmd = " /command yes     my      friend    ";
    UserInput user_input(cmd, ca);

    // INEXISTENT COMMAND
    assert (cmd == user_input.raw_input);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Command /command does not exist" == user_input.details);
    assert (4 == (int) user_input.parsed_input.size());
    assert ("/command" == user_input.parsed_input[0]);
    assert ("yes" == user_input.parsed_input[1]);
    assert ("my" == user_input.parsed_input[2]);
    assert ("friend" == user_input.parsed_input[3]);

    // BLANK
    cmd = "";
    user_input = UserInput(cmd, ca);
    assert ("Command cannot be blank" == user_input.details);
    assert (UserInput::INVALID == user_input.input_type);

    // DISCONNECT
    cmd = "/disconnect hahaha        hey oi? ?";
    user_input = UserInput(cmd, ca);
    assert (cmd == user_input.raw_input);
    assert ("" == user_input.details);
    assert (UserInput::DISCONNECT == user_input.input_type);
    assert (5 == (int) user_input.parsed_input.size());
    assert ("/disconnect" == user_input.parsed_input[0]);
    assert ("hahaha" == user_input.parsed_input[1]);
    assert ("hey" == user_input.parsed_input[2]);
    assert ("oi?" == user_input.parsed_input[3]);
    assert ("?" == user_input.parsed_input[4]);

    // JOIN ROOM
    ca.current_state = CHATTING;
    cmd = "/join 1";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("'join' command is not allowed when inside a room. You can leave this room by using '/leave' command" == user_input.details);
    assert (2 == (int) user_input.parsed_input.size());
    assert ("/join" == user_input.parsed_input[0]);
    assert ("1" == user_input.parsed_input[1]);

    ca.current_state = MENU;
    cmd = "/join 1 2 3";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Wrong usage of command 'join'. Usage: /join <room_identifier>" == user_input.details);

    cmd = "/join abacaxi";
    ca.current_state = MENU;
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Room identifier must be integer" == user_input.details);

    cmd = "/join 0";
    ca.current_state = MENU;
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("There is no room with identifier '0'" == user_input.details);

    ca.room_list.push_back(ChatRoom("room 1", 10, 10));
    ca.current_state = MENU;
    cmd = "/join 0";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Room '0' is full" == user_input.details);

    ca.room_list.push_back(ChatRoom("room 2", 1, 0));
    ca.current_state = MENU;
    cmd = "/join 1";
    user_input = UserInput(cmd, ca);
    assert (UserInput::JOIN_ROOM == user_input.input_type);
    assert (CHATTING == ca.current_state);
    assert (ca.room_list[1] == ca.current_room);

    ca.current_state = MENU;
    ca.room_list[0].current_participants = 0;
    cmd = "/join 0";
    user_input = UserInput(cmd, ca);
    assert (UserInput::JOIN_ROOM == user_input.input_type);
    assert (CHATTING == ca.current_state);
    assert (ca.room_list[0] == ca.current_room);

    // LEAVE ROOM
    ca.current_state = MENU;
    cmd = "/leave 1 abc abacaxi";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("'leave' should only be used when inside a room" == user_input.details);

    ca.current_state = CHATTING;
    cmd = "/leave 1 abc chinelo";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Wrong usage of command 'leave'. Usage: /leave" == user_input.details);

    ca.current_state = CHATTING;
    cmd = "/leave";
    user_input = UserInput(cmd, ca);
    assert (UserInput::LEAVE_ROOM == user_input.input_type);
    assert (MENU == ca.current_state);

    // CREATE ROOM
    ca.current_state = CHATTING;
    cmd = "/create room_1 10";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("'create' command is only allowed on the menu screen." == user_input.details);

    ca.current_state = MENU;
    cmd = "/create room 1 5";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Wrong usage of command 'create'. Usage: /create <room_name> <max_participants>" == user_input.details);

    ca.current_state = MENU;
    cmd = "/create aa 10";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Room name must be at least and at most 15 characters long." == user_input.details);

    ca.current_state = MENU;
    cmd = "/create aaaaaaaaaaaaaaaa 10";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Room name must be at least and at most 15 characters long." == user_input.details);

    ca.current_state = MENU;
    cmd = "/create room_1 abacaxi";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Max participants must be an integer" == user_input.details);

    ca.current_state = MENU;
    cmd = "/create room_1 0";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("A room can only admit at least 1 and at most 100 participants" == user_input.details);

    ca.current_state = MENU;
    cmd = "/create room_1 101";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("A room can only admit at least 1 and at most 100 participants" == user_input.details);

    ca.current_state = MENU;
    cmd = "/create room_1 100";
    user_input = UserInput(cmd, ca);
    assert (UserInput::CREATE_ROOM == user_input.input_type);

    ca.current_state = MENU;
    cmd = "/create room_1 100";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("There is already a room with this name: room_1" == user_input.details);

    ca.current_state = MENU;
    cmd = "/create room_2 1";
    user_input = UserInput(cmd, ca);
    assert (UserInput::CREATE_ROOM == user_input.input_type);

    // SEND MESSAGE
    ca.current_state = MENU;
    cmd = "Hey, what's up?";
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Invalid command" == user_input.details);

    ca.current_state = CHATTING;
    cmd = std::string(256, 'a');
    user_input = UserInput(cmd, ca);
    assert (UserInput::INVALID == user_input.input_type);
    assert ("Your message can be at most 255 characters long" == user_input.details);

    ca.current_state = CHATTING;
    cmd = std::string(255, 'a');
    user_input = UserInput(cmd, ca);
    assert (UserInput::SEND_MESSAGE == user_input.input_type);

    ca.current_state = CHATTING;
    cmd = "a";
    user_input = UserInput(cmd, ca);
    assert (UserInput::SEND_MESSAGE == user_input.input_type);

    std::cout << "Ok" << std::endl;
}

int main()
{
    chat_room_tests();
    user_input_handling_tests();
}