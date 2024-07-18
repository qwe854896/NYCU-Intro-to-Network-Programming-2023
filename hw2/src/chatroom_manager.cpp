#include "chatroom_manager.h"

#include "user_manager.h"

namespace jhc_hw2 {

ChatroomManager::ChatroomManager() {
    filtering_list.emplace_back("==");
    filtering_list.emplace_back("Superpie");
    filtering_list.emplace_back("hello");
    filtering_list.emplace_back("Starburst Stream");
    filtering_list.emplace_back("Domain Expansion");
}

void ChatroomManager::filter(std::string &message) {
    static auto case_insensitive_equal = [](const std::string &str1, const std::string &str2) {
        return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(), [](char a, char b)
                          { return tolower(a) == tolower(b); });
    };
    for (size_t i = 0; i < filtering_list.size(); ++i) {
        std::string &filtering_word = filtering_list[i];

        // If message contains filtering_word, then replace it with * of size filtering_word.size()
        // Use `case_insensitive_equal` to compare
        for (size_t j = 0; j < message.size(); ++j) {
            if (j + filtering_word.size() <= message.size() && case_insensitive_equal(message.substr(j, filtering_word.size()), filtering_word)) {
                for (size_t k = 0; k < filtering_word.size(); ++k) {
                    message[j + k] = '*';
                }
            }
        }
    }
}

int ChatroomManager::create_chatroom(int user_id, int chatroom_id) {
    if (chatroom_users.find(chatroom_id) != chatroom_users.end()) {
        return false;
    }
    chatroom_users[chatroom_id] = std::vector<int>();
    chatroom_messages[chatroom_id] = std::vector<std::string>();
    chatroom_owner[chatroom_id] = user_id;
    return true;
}

bool ChatroomManager::enter_chatroom(int user_id, int chatroom_id) {
    // If chatroom_id is not exist, then create it
    if (chatroom_users.find(chatroom_id) == chatroom_users.end()) {
        create_chatroom(user_id, chatroom_id);
    }
    chatroom_users[chatroom_id].emplace_back(user_id);
    return true;
}

bool ChatroomManager::leave_chatroom(int user_id, int chatroom_id) {
    if (chatroom_users.find(chatroom_id) == chatroom_users.end()) {
        return false;
    }
    auto &users = chatroom_users[chatroom_id];
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i] == user_id) {
            users.erase(users.begin() + i);
            return true;
        }
    }
    return false;
}

bool ChatroomManager::send_message(int user_id, int chatroom_id, std::string &message) {
    if (chatroom_users.find(chatroom_id) == chatroom_users.end()) {
        return false;
    }

    message = message.substr(0, MAX_MESSAGE_LENGTH);
    filter(message);

    std::string username = UserManager::get_instance().get_username(user_id);

    chatroom_messages[chatroom_id].emplace_back(username);
    chatroom_messages[chatroom_id].emplace_back(message);

    while (chatroom_messages[chatroom_id].size() > (MAX_NUM_MESSAGES << 1)) {
        chatroom_messages[chatroom_id].erase(chatroom_messages[chatroom_id].begin());
        chatroom_messages[chatroom_id].erase(chatroom_messages[chatroom_id].begin());
    }

    return true;
}

bool ChatroomManager::close_chatroom(int user_id, int chatroom_id) {
    if (chatroom_users.find(chatroom_id) == chatroom_users.end()) {
        return false;
    }
    if (chatroom_owner[chatroom_id] != user_id) {
        return false;
    }
    chatroom_users.erase(chatroom_id);
    chatroom_messages.erase(chatroom_id);
    chatroom_pin_message.erase(chatroom_id);
    chatroom_owner.erase(chatroom_id);
    return true;
}

bool ChatroomManager::get_chatroom_users(int chatroom_id, std::vector<int> &user_ids) {
    if (chatroom_users.find(chatroom_id) == chatroom_users.end()) {
        return false;
    }
    user_ids = chatroom_users[chatroom_id];
    return true;
}

bool ChatroomManager::get_chatroom_messages(int chatroom_id, std::vector<std::string> &messages) {
    if (chatroom_messages.find(chatroom_id) == chatroom_messages.end()) {
        return false;
    }
    messages = chatroom_messages[chatroom_id];
    return true;
}

bool ChatroomManager::get_chatroom_owner(int chatroom_id, int &user_id) {
    if (chatroom_owner.find(chatroom_id) == chatroom_owner.end()) {
        return false;
    }
    user_id = chatroom_owner[chatroom_id];
    return true;
}

bool ChatroomManager::get_all_chatrooms(std::vector<int> &chatroom_ids) {
    for (auto it = chatroom_users.begin(); it != chatroom_users.end(); ++it) {
        chatroom_ids.emplace_back(it->first);
    }
    return chatroom_ids.size();
}

bool ChatroomManager::get_pin_message(int chatroom_id, std::string &username, std::string &message) {
    if (chatroom_pin_message.find(chatroom_id) == chatroom_pin_message.end()) {
        return false;
    }
    username = chatroom_pin_message[chatroom_id].first;
    message = chatroom_pin_message[chatroom_id].second;
    return true;
}

bool ChatroomManager::set_pin_message(int chatroom_id, std::string username, std::string &message) {
    if (username == "") {
        chatroom_pin_message.erase(chatroom_id);
        return true;
    }

    message = message.substr(0, MAX_MESSAGE_LENGTH);
    filter(message);

    chatroom_pin_message[chatroom_id] = std::make_pair(username, message);
    return true;
}

bool ChatroomManager::get_chatroom(int chatroom_id) {
    if (chatroom_users.find(chatroom_id) == chatroom_users.end()) {
        return false;
    }
    return true;
}

} // namespace jhc_hw2