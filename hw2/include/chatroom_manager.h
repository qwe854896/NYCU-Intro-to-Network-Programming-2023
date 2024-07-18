#pragma once

#include <map>
#include <vector>
#include <string>

namespace jhc_hw2 {

constexpr const int MAX_MESSAGE_LENGTH = 150;
constexpr const int MAX_NUM_MESSAGES = 10;

class ChatroomManager {
private:
    std::map<int, std::vector<int>> chatroom_users;
    std::map<int, std::vector<std::string>> chatroom_messages;
    std::map<int, std::pair<std::string, std::string>> chatroom_pin_message;
    std::map<int, int> chatroom_owner;
    std::vector<std::string> filtering_list;
    ChatroomManager();
    void filter(std::string &message);

public:
    static ChatroomManager &get_instance() {
        static ChatroomManager instance;
        return instance;
    }
    ChatroomManager(ChatroomManager const &) = delete;
    void operator=(ChatroomManager const &) = delete;

    int create_chatroom(int user_id, int chatroom_id);
    bool enter_chatroom(int user_id, int chatroom_id);
    bool leave_chatroom(int user_id, int chatroom_id);
    bool send_message(int user_id, int chatroom_id, std::string &message);
    bool close_chatroom(int user_id, int chatroom_id);
    bool get_chatroom_users(int chatroom_id, std::vector<int> &user_ids);
    bool get_chatroom_messages(int chatroom_id, std::vector<std::string> &messages);
    bool get_chatroom_owner(int chatroom_id, int &user_id);
    bool get_all_chatrooms(std::vector<int> &chatroom_ids);
    bool get_pin_message(int chatroom_id, std::string &username, std::string &message);
    bool set_pin_message(int chatroom_id, std::string username, std::string &message);
    bool get_chatroom(int chatroom_id);
}; // class ChatroomManager

} // namespace jhc_hw2