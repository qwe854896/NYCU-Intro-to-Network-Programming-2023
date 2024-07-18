#pragma once

#include <string>

namespace jhc_hw2 {

constexpr const int MAX_COMMAND_LENGTH = 1024;
constexpr const int MAX_USERNAME_LENGTH = 20;
constexpr const int MAX_PASSWORD_LENGTH = 20;

class BasicCommandHandler {
public:
    BasicCommandHandler() = default;
    ~BasicCommandHandler() = default;

    bool HandleCommand(int sockfd, const char *command);

private:
    bool HandleRegister_(int sockfd, const std::string &username, const std::string &password);
    bool HandleLogin_(int sockfd, const std::string &username, const std::string &password);
    bool HandleLogout_(int sockfd);
    bool HandleExit_(int sockfd);
    bool HandleWhoami_(int sockfd);
    bool HandleSetStatus_(int sockfd, const std::string &status);
    bool HandleListUser_(int sockfd);
    bool HandleEnterChatRoom_(int sockfd, int number);
    bool HandleListChatRoom_(int sockfd);
    bool HandleCloseChatRoom_(int sockfd, int number);
    bool HandleUnknownCommand_(int sockfd);

}; // class BasicCommandHandler

class ChattingRoomCommandHandler {
public:
    ChattingRoomCommandHandler() = default;
    ~ChattingRoomCommandHandler() = default;

    bool HandleCommand(int sockfd, const char *command);

private:
    bool HandlePinMessage_(int sockfd, std::string &message);
    bool HandleDeletePinMessage_(int sockfd);
    bool HandleExitChatRoom_(int sockfd);
    bool HandleListUser_(int sockfd);
    bool HandleChatMessage_(int sockfd, std::string &message);
    bool HandleUnknownCommand_(int sockfd);

}; // class ChattingRoomCommandHandler

} // namespace jhc_hw2
