#include "ClientAttributes.hpp"

void ClientAttributes::push_message(std::string message)
{
    if ((int) messages.size() >= max_messages_on_screen)
        messages.pop_front();
    messages.push_back(message);
}