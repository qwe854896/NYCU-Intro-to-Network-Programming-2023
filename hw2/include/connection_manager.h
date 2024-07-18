#pragma once

#include <map>

namespace jhc_hw2 {

enum class ConnectionStatus {
    Guest,
    Login,
    Chatting,
}; // enum class ConnectionStatus

class ConnectionManager {
private:
    std::map<int, ConnectionStatus> connection_status;
    std::map<int, int> user_id;
    std::map<int, int> user_sockfd;
    ConnectionManager() {}

public:
    static ConnectionManager &get_instance() {
        static ConnectionManager instance;
        return instance;
    }
    ConnectionManager(ConnectionManager const &) = delete;
    void operator=(ConnectionManager const &) = delete;

    ConnectionStatus get_connection_status(int client_sockfd);
    bool logout(int client_sockfd);
    bool login(int client_sockfd, int user_id);
    int get_user_id(int client_sockfd);
    int get_user_sockfd(int user_id);
    bool enter_chatting_room(int client_sockfd);
    bool leave_chatting_room(int client_sockfd);
}; // class ConnectionManager

} // namespace jhc_hw2