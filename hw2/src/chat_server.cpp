#include "chat_server.h"

#include "socket_util.h"
#include "connection_manager.h"
#include "command_handler.h"

#include <cstring>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>

namespace jhc_hw2 {

ChatServer::~ChatServer() {
    Stop();
}

void ChatServer::Start(int port) {
    Init_(port);
    EpollWait_();
}

void ChatServer::Stop() {
    close(listenfd_);
    close(epfd_);
}

void ChatServer::Init_(int port) {
    // Create socket
    listenfd_ = socket_util::Socket(AF_INET, SOCK_STREAM, 0);
    
    // Set socket options
    int val = 1;
    socket_util::Setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

    // Bind
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    socket_util::Bind(listenfd_, (struct sockaddr *)&server_addr, sizeof(server_addr));
    socket_util::Listen(listenfd_, SOMAXCONN);

    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    // Create epoll
    epfd_ = epoll_create1(0);
    if (epfd_ == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Add server socket to epoll
    EpollRegister_(listenfd_);

    // Allocate events
    events_.resize(MAX_EVENTS);
}

void ChatServer::EpollWait_() {
    for (;;) {
        int num_events = epoll_wait(epfd_, events_.data(), MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        EpollHandleEvents_(num_events);
    }
}

void ChatServer::EpollHandleEvents_(int num) {
    for (int i = 0; i < num; ++i) {
        if (events_[i].data.fd == listenfd_) {
            EpollHandleAccept_();
        } else {
            EpollHandleData_(i);
        }
    }
}

void ChatServer::EpollHandleAccept_() {
    int client_sockfd = socket_util::Accept(listenfd_, NULL, NULL);
    EpollRegister_(client_sockfd);

    SendWelcomeMessage_(client_sockfd);
    SendPrompt_(client_sockfd);
}

void ChatServer::EpollHandleData_(int i) {
    int client_sockfd = events_[i].data.fd;
    if (ConnectionHandler(client_sockfd)) {
        close(client_sockfd);
        epoll_ctl(epfd_, EPOLL_CTL_DEL, client_sockfd, NULL);
    } else {
        SendPrompt_(client_sockfd);
    }
}

void ChatServer::EpollRegister_(int sockfd) {
    struct epoll_event event;
    event.data.fd = sockfd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, sockfd, &event) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
}

void ChatServer::SendWelcomeMessage_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    sprintf(buf, "*********************************\n** Welcome to the Chat server. **\n*********************************\n");
    socket_util::Write(client_sockfd, buf, strlen(buf));
}

void ChatServer::SendPrompt_(int client_sockfd) {
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static char buf[MAX_COMMAND_LENGTH];

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Chatting) {
        return;
    }

    sprintf(buf, "%% ");
    socket_util::Write(client_sockfd, buf, strlen(buf));
}

// If return true, then this client should be removed from the client list
bool ChatServer::ConnectionHandler(int client_sockfd) {
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static BasicCommandHandler basic_command_handler;
    static ChattingRoomCommandHandler chatting_room_handler;
    static char read_buf[MAX_COMMAND_LENGTH];

    socket_util::ReadLine(client_sockfd, read_buf, MAX_COMMAND_LENGTH);

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Chatting) {
        return chatting_room_handler.HandleCommand(client_sockfd, read_buf);
    } else {
        return basic_command_handler.HandleCommand(client_sockfd, read_buf);
    }
}

} // namespace jhc_hw2