#ifndef _CHAT_ROOM_HPP_
#define _CHAT_ROOM_HPP_

#include <string>
#include <vector>
#include <utility>
#include <set>

class ChatRoom
{
public:
    ChatRoom();
    ~ChatRoom();
    ChatRoom(std::string name, int max_participants, int current_participants);
    ChatRoom(std::string serialized_chat_room);
    std::string name;
    int max_participants;
    int current_participants;

    bool operator==(ChatRoom & cr);

    std::string to_string() const;
    static std::string serialize_room_list(std::vector<ChatRoom> & room_list);
    static std::vector<ChatRoom> deserialize_room_list(std::string & room_list);

    static bool name_taken(std::string t_name);
private:
    static std::set<std::string> rooms_names;
};

#endif