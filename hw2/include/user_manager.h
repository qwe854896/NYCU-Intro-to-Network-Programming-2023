#pragma once

#include <map>
#include <string>
#include <vector>

namespace jhc_hw2 {

enum class UserStatus {
    Online,
    Offline,
    Busy,
}; // enum class UserStatus

class UserManager {
private:
    std::map<std::string, std::string> user_password;
    std::map<std::string, int> user_id;
    std::map<int, std::string> id_user;
    std::map<int, UserStatus> user_status;
    std::map<int, int> user_chatroom_id;
    int next_id = 1;
    UserManager() {}

public:
    static UserManager &get_instance() {
        static UserManager instance;
        return instance;
    }
    UserManager(UserManager const &) = delete;
    void operator=(UserManager const &) = delete;

    bool register_user(std::string username, std::string password);
    bool login(std::string username, std::string password);
    bool logout(int user_id);
    int get_user_id(std::string username);
    std::string get_username(int user_id);
    UserStatus get_user_status(int user_id);
    bool set_user_status(int user_id, UserStatus status);
    int get_all_users(std::vector<int> &user_ids);
    bool enter_chatroom(int user_id, int chatroom_id);
    bool leave_chatroom(int user_id, int chatroom_id);
    int get_chatroom_id(int user_id);
}; // class UserManager

} // namespace jhc_hw2