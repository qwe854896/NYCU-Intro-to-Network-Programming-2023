#pragma once

#include <sys/epoll.h>
#include <vector>

namespace jhc_hw2 {

constexpr const int MAX_EVENTS = 10;

class ChatServer {
public:
    ChatServer() = default;
    ~ChatServer();

    void Start(int port);
    void Stop();

private:
    int listenfd_;
    int epfd_;
    std::vector<struct epoll_event> events_;

    void Init_(int port);
    void EpollWait_();
    void EpollHandleEvents_(int num);
    void EpollHandleAccept_();
    void EpollHandleData_(int i);
    void EpollRegister_(int sockfd);

    void SendWelcomeMessage_(int sockfd);
    void SendPrompt_(int sockfd);

    bool ConnectionHandler(int sockfd);
}; // class ChatServer

} // namespace ChatServer