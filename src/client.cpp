#include <iostream>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <string.h>
#include <errno.h>

#include "Communication.hpp"
#include "ChatRoom.hpp"
#include "ClientAttributes.hpp"
#include "UserInputHandling.hpp"

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 3000

std::string get_nickname()
{
    std::string nick;
    while (true) {
        printf("Enter a nickname: ");
        std::getline(std::cin, nick);

        if (nick.size() > 15 || nick.size() < 3) {
            printf("\tYour nickname must have at most 15 and at least 3 characters!\n");
        } else {
            break;
        }
    }
    return nick;
}

void handle_user_input(ClientAttributes & client_attributes)
{
    std::string cmd;
    std::getline(std::cin, cmd);
    UserInput user_input(cmd, client_attributes);
    client_attributes.info_message = user_input.details;
}

void draw_gui_header(ClientAttributes & client_attributes)
{
    printf(" __________________________________________ CPP CHAT __________________________________________\n");
    printf("/                                                                                              \\\n");
    if (!client_attributes.info_message.empty())
        printf("| INFO: \033[0;33m%s\033[0m\n", client_attributes.info_message.c_str());
    else
        printf("|\n");
}

void draw_chat_message(std::string chat_message)
{
    printf("%s\n", chat_message.c_str());
}

void draw_room_list(std::vector<ChatRoom> & room_list)
{
    std::cout << "| Available rooms:\n|\n";
    for (int i = 0; i < (int) room_list.size(); i++) {
        printf("|\t(id: %d) %s [%d/%d]\n", i, room_list[i].name.c_str(), room_list[i].current_participants, room_list[i].max_participants);
    }
    for (int i = 0; i < (int) std::max(0, int(20 - room_list.size())); i++)
        printf("|\n");

    printf("|_______________________________________________________________________________________________\n");
    std::cout << "| Commands:\n|\t/join <room_id> - to join a room\n"
        << "|\t/create <room_name> <max_participants> - to create a new room (name cannot have white spaces)\n"
        << "|\t/disconnect - to disconnect from CPP CHAT" << std::endl;
    std::cout << "|_______________________________________________________________________________________________\n";
}

void draw_messages(ClientAttributes & client_attributes)
{
    int cur_participants = 0, max_participants = 0;
    for (auto room : client_attributes.room_list) {
        if (room.name == client_attributes.current_room.name) {
            cur_participants = room.current_participants;
            max_participants = room.max_participants;
        }
    }
    std::cout << "| " << client_attributes.current_room.name << " [" << cur_participants << "/" << max_participants << "]\n";
    std::cout << "|\t\t\t\t\tMessages:" << std::endl;
    printf("|_______________________________________________________________________________________________\n");

    for (int i = 0; i < (int) std::max(0, int(20 - client_attributes.messages.size())); i++)
        printf("|\n");
    for (auto e : client_attributes.messages)
        std::cout << "|\t" + e << std::endl;
    printf("|_______________________________________________________________________________________________\n");
    std::cout << "| Commands:\n|\t/leave - to leave this room\n"
        << "|\t/disconnect - to disconnect from CPP CHAT" << std::endl;
    std::cout << "|_______________________________________________________________________________________________\n";
}

void update_screen(ClientAttributes & client_attributes)
{
    system("tput reset");
    draw_gui_header(client_attributes);
    if (client_attributes.current_state == MENU) {
        draw_room_list(client_attributes.room_list);
    }
    else if (client_attributes.current_state == CHATTING) {
        draw_messages(client_attributes);
    }
}

int main()
{
    ClientAttributes client;
    client.current_state = MENU;
    client.max_messages_on_screen = 19;

    client.nickname = get_nickname();

    client.communication_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client.communication_socket_fd == 0) {
        std::cerr << "Could not create the communication socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address. Address not supported" << std::endl;
        exit(1);
    }

    if (connect(client.communication_socket_fd, (sockaddr *) &server_address, sizeof(server_address)) < 0) {
        std::cerr << "Unable to connect to server " << SERVER_ADDRESS << ":" << SERVER_PORT << ". Error: " << strerror(errno) << std::endl;
        exit(1);
    }

    send(client.communication_socket_fd, client.nickname.c_str(), client.nickname.size(), 0);

    fd_set io_and_socket;
    FD_ZERO(&io_and_socket);

    FD_SET(0, &io_and_socket);
    FD_SET(client.communication_socket_fd, &io_and_socket);

    while (client.current_state != DISCONNECTED) {
        update_screen(client);

        fd_set ready_fd = io_and_socket;
        select(FD_SETSIZE, &ready_fd, nullptr, nullptr, nullptr);

        if (FD_ISSET(0, &ready_fd)) {
            handle_user_input(client);
        }
        if (FD_ISSET(client.communication_socket_fd, &ready_fd)) {
            CommunicationData data = Communication::receive_data(client.communication_socket_fd);
            if (data.data_type == Communication::CHAT_MESSAGE)
                client.push_message(data.data_content);
            else if (data.data_type == Communication::ROOM_LIST)
                client.room_list = ChatRoom::deserialize_room_list(data.data_content);
        }
    }

    close(client.communication_socket_fd);

    return 0;
}