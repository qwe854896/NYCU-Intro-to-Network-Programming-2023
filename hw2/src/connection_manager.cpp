#include "connection_manager.h"

namespace jhc_hw2 {

ConnectionStatus ConnectionManager::get_connection_status(int client_sockfd) {
    if (connection_status.find(client_sockfd) == connection_status.end()) {
        connection_status[client_sockfd] = ConnectionStatus::Guest;
    }
    return connection_status[client_sockfd];
}

bool ConnectionManager::logout(int client_sockfd) {
    connection_status.erase(client_sockfd);
    user_sockfd.erase(user_id[client_sockfd]);
    user_id.erase(client_sockfd);
    return true;
}

bool ConnectionManager::login(int client_sockfd, int user_id) {
    if (user_sockfd.find(user_id) != user_sockfd.end()) {
        return false;
    }
    connection_status[client_sockfd] = ConnectionStatus::Login;
    this->user_id[client_sockfd] = user_id;
    user_sockfd[user_id] = client_sockfd;
    return true;
}

int ConnectionManager::get_user_id(int client_sockfd) {
    if (user_id.find(client_sockfd) == user_id.end()) {
        return -1;
    }
    return user_id[client_sockfd];
}

int ConnectionManager::get_user_sockfd(int user_id) {
    if (user_sockfd.find(user_id) == user_sockfd.end()) {
        return -1;
    }
    return user_sockfd[user_id];
}

bool ConnectionManager::enter_chatting_room(int client_sockfd) {
    connection_status[client_sockfd] = ConnectionStatus::Chatting;
    return true;
}

bool ConnectionManager::leave_chatting_room(int client_sockfd) {
    connection_status[client_sockfd] = ConnectionStatus::Login;
    return true;
}

} // namespace jhc_hw2