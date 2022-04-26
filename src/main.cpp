#include <iostream>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

#include <sys/epoll.h>

#define LISTENING_PORT 3000
#define MAX_CONNECTIONS 1000

int main()
{
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
        std::cerr << "Could not bind socke to port " << LISTENING_PORT << ": " << strerror(errno) << std::endl;
        exit(1);
    }

    listen(listening_socket_fd, MAX_CONNECTIONS);

    fd_set sockets_fd_set;
    FD_SET(listening_socket_fd, &sockets_fd_set);

    bool server_is_running = true;
    while (server_is_running) {
        fd_set temp_sockets_fd_set = sockets_fd_set;
        int sockets_amount = select(0, &temp_sockets_fd_set, nullptr, nullptr, nullptr);
        if (sockets_amount == -1) {
            std::cerr << "Woops, something went wrong while listening to sockets: " << strerror(errno) << std::endl;
        }

    }

    struct sockaddr_in client_socket_address;
    int client_socket_address_len = sizeof(client_socket_address);
    int client_socket = accept(listening_socket_fd, (struct sockaddr *) &client_socket_address, (socklen_t *) &client_socket_address_len);

    std::cout << client_socket_address.sin_port << std::endl;

    char buffer[5024];

    int epoll_fd = epoll_create(0);
    if (epoll_fd == -1) {
        std::cerr << "Unable to create an epoll"
    }

    while (true) {
        int data_read = read(client_socket, buffer, 5024);

        if (data_read == 0) {
            std::cout << "Goodbye! :)" << std::endl;
            break;
        }

        std::cout << buffer << std::endl;
        memset(buffer, 0, 5024);
    }

    close(listening_socket_fd);

    return 0;
}