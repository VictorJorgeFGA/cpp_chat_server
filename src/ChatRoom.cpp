#include "ChatRoom.hpp"

std::set<std::string> ChatRoom::rooms_names;

ChatRoom::ChatRoom():
name("0"),
max_participants(0),
current_participants(0)
{

}

ChatRoom::ChatRoom(std::string t_name, int t_max_participants, int t_current_participants):
name(t_name),
max_participants(t_max_participants),
current_participants(t_current_participants)
{
    rooms_names.insert(t_name);
}

ChatRoom::ChatRoom(std::string serialized_chat_room)
{
    auto it = serialized_chat_room.begin();
    for ( ; *it != ';'; it++) name.push_back(*it);
    it++;

    std::string mp;
    for ( ; *it != ';'; it++) mp.push_back(*it);
    it++;

    std::string cp;
    for ( ; *it != ';'; it++) cp.push_back(*it);
    it++;

    max_participants = std::stoi(mp);
    current_participants = std::stoi(cp);

    rooms_names.insert(name);
}

ChatRoom::~ChatRoom()
{

}

bool ChatRoom::operator==(ChatRoom & cr)
{
    return (name == cr.name && max_participants == cr.max_participants && current_participants == cr.current_participants);
}

std::string ChatRoom::to_string() const
{
    return name + ";" + std::to_string(max_participants) + ";" + std::to_string(current_participants) + ";";
}

std::string ChatRoom::serialize_room_list(std::vector<ChatRoom> & room_list)
{
    std::string serialized;
    for (auto & cr : room_list)
        serialized += cr.to_string();
    return serialized;
}

std::vector<ChatRoom> ChatRoom::deserialize_room_list(std::string & room_list)
{
    std::vector<ChatRoom> deserialized;

    auto it = room_list.begin();
    while (true) {
        if (it == room_list.end())
            break;

        std::string n;
        for ( ; *it != ';'; it++) n.push_back(*it);
        it++;

        std::string mp;
        for ( ; *it != ';'; it++) mp.push_back(*it);
        it++;

        std::string cp;
        for ( ; *it != ';'; it++) cp.push_back(*it);
        it++;

        deserialized.push_back(ChatRoom(n, std::stoi(mp), std::stoi(cp)));
    }
    return deserialized;
}

bool ChatRoom::name_taken(std::string t_name)
{
    return rooms_names.count(t_name) > 0;
}