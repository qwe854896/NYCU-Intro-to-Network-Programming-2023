#include "user_manager.h"

namespace jhc_hw2 {

bool UserManager::register_user(std::string username, std::string password) {
    if (user_password.find(username) != user_password.end()) {
        return false;
    }
    user_password[username] = password;
    user_id[username] = next_id;
    id_user[next_id] = username;
    ++next_id;
    return true;
}

bool UserManager::login(std::string username, std::string password) {
    if (user_password.find(username) == user_password.end()) {
        return false;
    }
    if (user_password[username] != password) {
        return false;
    }
    user_status[user_id[username]] = UserStatus::Online;
    return true;
}

bool UserManager::logout(int user_id) {
    if (id_user.find(user_id) == id_user.end()) {
        return false;
    }
    user_status[user_id] = UserStatus::Offline;
    return true;
}

int UserManager::get_user_id(std::string username) {
    if (user_id.find(username) == user_id.end()) {
        return -1;
    }
    return user_id[username];
}

std::string UserManager::get_username(int user_id) {
    if (id_user.find(user_id) == id_user.end()) {
        return "";
    }
    return id_user[user_id];
}

UserStatus UserManager::get_user_status(int user_id) {
    if (user_status.find(user_id) == user_status.end()) {
        return UserStatus::Offline;
    }
    return user_status[user_id];
}

bool UserManager::set_user_status(int user_id, UserStatus status) {
    if (user_status.find(user_id) == user_status.end()) {
        return false;
    }
    user_status[user_id] = status;
    return true;
}

int UserManager::get_all_users(std::vector<int> &user_ids) {
    for (auto it = user_id.begin(); it != user_id.end(); ++it) {
        user_ids.emplace_back(it->second);
    }
    return user_ids.size();
}

bool UserManager::enter_chatroom(int user_id, int chatroom_id) {
    if (user_chatroom_id.find(user_id) != user_chatroom_id.end()) {
        return false;
    }
    user_chatroom_id[user_id] = chatroom_id;
    return true;
}

bool UserManager::leave_chatroom(int user_id, int chatroom_id) {
    if (user_chatroom_id.find(user_id) == user_chatroom_id.end()) {
        return false;
    }
    user_chatroom_id.erase(user_id);
    return true;
}

int UserManager::get_chatroom_id(int user_id) {
    if (user_chatroom_id.find(user_id) == user_chatroom_id.end()) {
        return -1;
    }
    return user_chatroom_id[user_id];
}

} // namespace jhc_hw2