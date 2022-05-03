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

#include <vector>
#include <utility>
#include <map>

#include "Communication.hpp"
#include "ChatRoom.hpp"

#define LISTENING_PORT 3000
#define MAX_CONNECTIONS 1000

struct User
{
    int socket_fd;
    std::string nickname;
    ChatRoom current_room;
    sockaddr_in socket_address;
};

void deliver_chat_message(std::string chat_message, User src, std::vector<User> & dst)
{
    for (auto u : dst) {
        if (u.socket_fd == src.socket_fd)
            Communication::send_chat_message(u.socket_fd, "You: " + chat_message);
        else if (u.current_room.name == src.current_room.name)
            Communication::send_chat_message(u.socket_fd, src.nickname + ": " + chat_message);
    }
}

void send_updated_rooms_list(std::vector<User> users, std::vector<ChatRoom> rooms)
{
    std::string s_rooms = ChatRoom::serialize_room_list(rooms);
    for (auto user : users)
        Communication::send_room_list(user.socket_fd, s_rooms);
}

void update_room_participants(std::string room_name, Communication::DataType movement, std::vector<ChatRoom> & rooms)
{
    for (auto & room : rooms) {
        if (room_name == room.name) {
            if (Communication::LEAVE_ROOM == movement)
                room.current_participants--;
            else if (Communication::JOIN_ROOM == movement)
                room.current_participants++;
        }
    }
}

int main()
{
    std::cout << "Initializing server..." << std::endl;

    int listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_socket_fd == 0) {
        std::cerr << "Could not create the listening socket: " << strerror(errno) << std::endl;
        exit(1);
    }

    struct sockaddr_in listening_socket_address;
    listening_socket_address.sin_family = AF_INET;
    listening_socket_address.sin_addr.s_addr = INADDR_ANY;
    listening_socket_address.sin_port = htons(LISTENING_PORT);

    if (bind(listening_socket_fd, (struct sockaddr *) &listening_socket_address, sizeof(listening_socket_address)) < 0) {
        std::cerr << "Could not bind to the socket to port " << LISTENING_PORT << ": " << strerror(errno) << std::endl;
        exit(1);
    }
    std::cout << "Bind done! Listening now on port " << LISTENING_PORT << std::endl;

    listen(listening_socket_fd, MAX_CONNECTIONS);

    fd_set sockets_fd_set;
    FD_ZERO(&sockets_fd_set);
    FD_SET(listening_socket_fd, &sockets_fd_set);

    std::vector<int> all_sockets;

    std::vector<User> users;
    auto user_to_remove = users.end();
    bool need_to_remove_user = false;
    std::vector<ChatRoom> rooms { ChatRoom("room_0", 5, 0) };

    bool server_is_running = true;
    while (server_is_running) {
        fd_set temp_sockets_fd_set = sockets_fd_set;

        // remover users
        if (need_to_remove_user) {
            update_room_participants(user_to_remove->current_room.name, Communication::LEAVE_ROOM, rooms);
            users.erase(user_to_remove);
            send_updated_rooms_list(users, rooms);
            need_to_remove_user = false;
        }

        std::cout << "There are currently " << users.size() << " users connected" << std::endl;

        // Otimizar trocando FD_SETSIZE para max(all_sockets)
        int sockets_amount = select(FD_SETSIZE, &temp_sockets_fd_set, nullptr, nullptr, nullptr);

        if (sockets_amount == -1) {
            std::cerr << "Woops, something went wrong while listening to sockets: " << strerror(errno) << std::endl;
        }

        // Accepts new connections
        if (FD_ISSET(listening_socket_fd, &temp_sockets_fd_set)) {
            std::cout << "A new user is trying to stablish connection..." << std::endl;
            char communication_buffer[4096];
            struct User new_user;
            socklen_t new_user_addr_len = sizeof(new_user.socket_address);
            new_user.socket_fd = accept(listening_socket_fd, (struct sockaddr*) &new_user.socket_address, &new_user_addr_len);
            std::cout << "Address: " << inet_ntoa(new_user.socket_address.sin_addr) << ":" << new_user.socket_address.sin_port << std::endl;

            FD_SET(new_user.socket_fd, &sockets_fd_set);
            all_sockets.push_back(new_user.socket_fd);

            memset(communication_buffer, 0, sizeof(communication_buffer));
            recv(new_user.socket_fd, communication_buffer, sizeof(communication_buffer), 0);
            new_user.nickname = communication_buffer;

            users.push_back(new_user);
            std::cout << "Just got a new connection from user \"" << new_user.nickname << "\"" << std::endl;
            Communication::send_room_list(new_user.socket_fd, ChatRoom::serialize_room_list(rooms));
        }

        // Check for new messages
        for (int i = 0; i < (int) users.size(); i++) {
            auto & cur_user = users[i];
            int cur_fd = cur_user.socket_fd;

            if (FD_ISSET(cur_fd, &temp_sockets_fd_set)) {
                CommunicationData communication_data = Communication::receive_data(cur_fd);
                std::cout << cur_user.nickname << " sent: ";

                switch (communication_data.data_type)
                {
                case Communication::DISCONNECT:
                    std::cout << "DISCONNECT" << std::endl;
                    for (auto it = users.begin(); it != users.end(); it++) {
                        if (it->nickname == cur_user.nickname) {
                            user_to_remove = it;
                            break;
                        }
                    }
                    close(cur_user.socket_fd);
                    FD_CLR(cur_user.socket_fd, &sockets_fd_set);
                    need_to_remove_user = true;
                    break;
                case Communication::JOIN_ROOM:
                    std::cout << "JOIN_ROOM" << std::endl;
                    cur_user.current_room = rooms.at(std::stoi(communication_data.data_content));
                    update_room_participants(cur_user.current_room.name, Communication::JOIN_ROOM, rooms);
                    send_updated_rooms_list(users, rooms);
                    break;
                case Communication::LEAVE_ROOM:
                    std::cout << "LEAVE_ROOM" << std::endl;
                    update_room_participants(cur_user.current_room.name, Communication::LEAVE_ROOM, rooms);
                    send_updated_rooms_list(users, rooms);
                    cur_user.current_room = ChatRoom();
                    break;
                case Communication::CHAT_MESSAGE:
                    std::cout << "CHAT_MESSAGE" << std::endl;
                    deliver_chat_message(communication_data.data_content, cur_user, users);
                    break;
                case Communication::CREATE_ROOM:
                    std::cout << "CREATE_ROOM" << std::endl;
                    rooms.push_back(ChatRoom(communication_data.data_content));
                    send_updated_rooms_list(users, rooms);
                    break;
                default:
                    break;
                }
            }
        }
    }

    close(listening_socket_fd);

    return 0;
}