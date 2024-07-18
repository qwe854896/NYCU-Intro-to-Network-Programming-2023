#include "command_handler.h"

#include "socket_util.h"
#include "user_manager.h"
#include "connection_manager.h"
#include "chatroom_manager.h"

#include <cstring>
#include <algorithm>
#include <sstream>

namespace jhc_hw2 {

bool BasicCommandHandler::HandleCommand(int client_sockfd, const char *command) {
    static char buf[MAX_COMMAND_LENGTH];

    std::string op;
    std::vector<std::string> args;

    std::stringstream ss(command);
    ss >> op;
    std::string arg;
    while (ss >> arg) {
        args.emplace_back(arg);
    }

    if ("register" == op || "r" == op) {
        if (args.size() != 2) {
            snprintf(buf, sizeof(buf), "Usage: register <username> <password>\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleRegister_(client_sockfd, args[0], args[1]);

    } else if ("login" == op || "li" == op) {
        if (args.size() != 2) {
            snprintf(buf, sizeof(buf), "Usage: login <username> <password>\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleLogin_(client_sockfd, args[0], args[1]);

    } else if ("logout" == op || "lo" == op) {
        if (args.size() != 0) {
            snprintf(buf, sizeof(buf), "Usage: logout\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleLogout_(client_sockfd);

    } else if ("exit" == op) {
        if (args.size() != 0) {
            snprintf(buf, sizeof(buf), "Usage: exit\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleExit_(client_sockfd);

    } else if ("whoami" == op) {
        if (args.size() != 0) {
            snprintf(buf, sizeof(buf), "Usage: whoami\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleWhoami_(client_sockfd);

    } else if ("set-status" == op) {
        if (args.size() != 1) {
            snprintf(buf, sizeof(buf), "Usage: set-status <status>\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleSetStatus_(client_sockfd, args[0]);

    } else if ("list-user" == op) {
        if (args.size() != 0) {
            snprintf(buf, sizeof(buf), "Usage: list-user\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleListUser_(client_sockfd);

    } else if ("enter-chat-room" == op || "e" == op) {
        if (args.size() != 1) {
            snprintf(buf, sizeof(buf), "Usage: enter-chat-room <number>\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        if (args[0].find_first_not_of("0123456789") != std::string::npos) {
            snprintf(buf, sizeof(buf), "Number %s is not valid.\n", args[0].c_str());
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        int number = atoi(args[0].c_str());
        if (number < 1 || number > 100) {
            snprintf(buf, sizeof(buf), "Number %s is not valid.\n", args[0].c_str());
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleEnterChatRoom_(client_sockfd, number);

    } else if ("list-chat-room" == op || "l" == op) {
        if (args.size() != 0) {
            snprintf(buf, sizeof(buf), "Usage: list-chat-room\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleListChatRoom_(client_sockfd);

    } else if ("close-chat-room" == op || "c" == op) {
        if (args.size() != 1) {
            snprintf(buf, sizeof(buf), "Usage: close-chat-room <number>\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        if (args[0].find_first_not_of("0123456789") != std::string::npos) {
            snprintf(buf, sizeof(buf), "Number %s is not valid.\n", args[0].c_str());
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        int number = atoi(args[0].c_str());
        if (number < 1 || number > 100) {
            snprintf(buf, sizeof(buf), "Number %s is not valid.\n", args[0].c_str());
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }
        return HandleCloseChatRoom_(client_sockfd, number);

    } else {
        snprintf(buf, sizeof(buf), "Error: Unknown command\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }
}

bool BasicCommandHandler::HandleRegister_(int client_sockfd, const std::string &username, const std::string &password) {
    static char buf[MAX_COMMAND_LENGTH];
    static UserManager& user_manager = UserManager::get_instance();

    if (user_manager.register_user(username, password)) {
        snprintf(buf, sizeof(buf), "Register successfully.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    } else {
        snprintf(buf, sizeof(buf), "Username is already used.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }
}

bool BasicCommandHandler::HandleLogin_(int client_sockfd, const std::string &username, const std::string &password) {
    static char buf[MAX_COMMAND_LENGTH];
    static UserManager& user_manager = UserManager::get_instance();
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Login) {
        snprintf(buf, sizeof(buf), "Please logout first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    if (user_manager.login(username, password)) {
        int user_id = user_manager.get_user_id(username);

        if (connection_manager.login(client_sockfd, user_id)) {
            snprintf(buf, sizeof(buf), "Welcome, %s.\n", username.c_str());
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        } else {
            snprintf(buf, sizeof(buf), "Login failed.\n");
            socket_util::Write(client_sockfd, buf, strlen(buf));
            return false;
        }

    } else {
        snprintf(buf, sizeof(buf), "Login failed.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }
}

bool BasicCommandHandler::HandleLogout_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    int user_id = connection_manager.get_user_id(client_sockfd);

    if (connection_manager.logout(client_sockfd)) {
        std::string username = user_manager.get_username(user_id);
        snprintf(buf, sizeof(buf), "Bye, %s.\n", username.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));

        user_manager.logout(user_id);

        return false;
    } else {
        snprintf(buf, sizeof(buf), "Logout failed.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }
}

bool BasicCommandHandler::HandleExit_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Login) {
        int user_id = connection_manager.get_user_id(client_sockfd);
        std::string username = user_manager.get_username(user_id);
        snprintf(buf, sizeof(buf), "Bye, %s.\n", username.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));

        user_manager.logout(user_id);
        connection_manager.logout(client_sockfd);
    }

    return true;
}

bool BasicCommandHandler::HandleWhoami_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    snprintf(buf, sizeof(buf), "%s\n", username.c_str());
    socket_util::Write(client_sockfd, buf, strlen(buf));
    return false;
}

bool BasicCommandHandler::HandleSetStatus_(int client_sockfd, const std::string &status) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    int user_id = connection_manager.get_user_id(client_sockfd);

    UserStatus user_status;
    if (status == "online") {
        user_status = UserStatus::Online;
    } else if (status == "offline") {
        user_status = UserStatus::Offline;
    } else if (status == "busy") {
        user_status = UserStatus::Busy;
    } else {
        snprintf(buf, sizeof(buf), "set-status failed\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    if (user_manager.set_user_status(user_id, user_status)) {
        std::string username = user_manager.get_username(user_id);
        snprintf(buf, sizeof(buf), "%s %s\n", username.c_str(), status.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    } else {
        snprintf(buf, sizeof(buf), "set-status failed\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }
}

bool BasicCommandHandler::HandleListUser_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    // Print all users sorted by username
    std::vector<int> user_ids;
    user_manager.get_all_users(user_ids);

    std::sort(user_ids.begin(), user_ids.end(), [&](int a, int b) {
        return user_manager.get_username(a) < user_manager.get_username(b);
    });

    for (size_t i = 0; i < user_ids.size(); ++i) {
        int user_id = user_ids[i];
        std::string username = user_manager.get_username(user_id);
        UserStatus user_status = user_manager.get_user_status(user_id);
        std::string status;
        if (user_status == UserStatus::Online) {
            status = "online";
        } else if (user_status == UserStatus::Offline) {
            status = "offline";
        } else if (user_status == UserStatus::Busy) {
            status = "busy";
        }
        snprintf(buf, sizeof(buf), "%s %s\n", username.c_str(), status.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));
    }

    return false;
}

bool BasicCommandHandler::HandleEnterChatRoom_(int client_sockfd, int number) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);

    connection_manager.enter_chatting_room(client_sockfd);
    user_manager.enter_chatroom(user_id, number);
    chatroom_manager.enter_chatroom(user_id, number);

    std::vector<int> chatroom_users;
    chatroom_manager.get_chatroom_users(number, chatroom_users);
    int chatroom_owner;
    chatroom_manager.get_chatroom_owner(number, chatroom_owner);
    std::vector<std::string> chatroom_messages;
    chatroom_manager.get_chatroom_messages(number, chatroom_messages);
    
    snprintf(buf, sizeof(buf), "Welcome to the public chat room.\n");
    socket_util::Write(client_sockfd, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "Room number: %d\n", number);
    socket_util::Write(client_sockfd, buf, strlen(buf));
    snprintf(buf, sizeof(buf), "Owner: %s\n", user_manager.get_username(chatroom_owner).c_str());
    socket_util::Write(client_sockfd, buf, strlen(buf));
    for (size_t i = 0; i < chatroom_messages.size(); i += 2) {
        std::string username = chatroom_messages[i];
        std::string message = chatroom_messages[i + 1];
        snprintf(buf, sizeof(buf), "[%s]: %s\n", username.c_str(), message.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));
    }

    // Send message to other users:
    // <username> had enter the chat room.
    for (size_t i = 0; i < chatroom_users.size(); ++i) {
        int user_id = chatroom_users[i];
        int sockfd = connection_manager.get_user_sockfd(user_id);
        if (sockfd != client_sockfd) {
            snprintf(buf, sizeof(buf), "%s had enter the chat room.\n", username.c_str());
            socket_util::Write(sockfd, buf, strlen(buf));
        }
    }

    // If there is a pin message in the chat room:
    // Pin -> [<username>]: <message>\n
    std::string message;
    if (chatroom_manager.get_pin_message(number, username, message)) {
        snprintf(buf, sizeof(buf), "Pin -> [%s]: %s\n", username.c_str(), message.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));
    }

    return false;
}

bool BasicCommandHandler::HandleListChatRoom_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    // For every chatroom:
    // <owner username> <room number>
    // Note that the chatrooms are sorted by room number
    std::vector<int> chatroom_ids;
    chatroom_manager.get_all_chatrooms(chatroom_ids);

    std::sort(chatroom_ids.begin(), chatroom_ids.end());

    for (size_t i = 0; i < chatroom_ids.size(); ++i) {
        int chatroom_id = chatroom_ids[i];
        int chatroom_owner;
        chatroom_manager.get_chatroom_owner(chatroom_id, chatroom_owner);
        std::string username = user_manager.get_username(chatroom_owner);
        snprintf(buf, sizeof(buf), "%s %d\n", username.c_str(), chatroom_id);
        socket_util::Write(client_sockfd, buf, strlen(buf));
    }

    return false;
}

bool BasicCommandHandler::HandleCloseChatRoom_(int client_sockfd, int number) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    if (connection_manager.get_connection_status(client_sockfd) == ConnectionStatus::Guest) {
        snprintf(buf, sizeof(buf), "Please login first.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    
    std::vector<int> chatroom_users;
    chatroom_manager.get_chatroom_users(number, chatroom_users);

    // If chat room does not exist:
    // Chat room <number> does not exist.
    if (!chatroom_manager.get_chatroom(number)) {
        snprintf(buf, sizeof(buf), "Chat room %d does not exist.\n", number);
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    // If user is not the owner of the chat room:
    // Only the owner can close this chat room.
    int chatroom_owner;
    chatroom_manager.get_chatroom_owner(number, chatroom_owner);
    if (chatroom_owner != user_id) {
        snprintf(buf, sizeof(buf), "Only the owner can close this chat room.\n");
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }
    
    // Send message to other users:
    // Chat room <number> was closed.
    for (size_t i = 0; i < chatroom_users.size(); ++i) {
        int user_id = chatroom_users[i];
        int sockfd = connection_manager.get_user_sockfd(user_id);
        snprintf(buf, sizeof(buf), "Chat room %d was closed.\n", number);
        socket_util::Write(sockfd, buf, strlen(buf));

        // Leave chat room
        connection_manager.leave_chatting_room(sockfd);
        user_manager.leave_chatroom(user_id, number);

        // Print prompt for kicked user
        sprintf(buf, "%% ");
        socket_util::Write(sockfd, buf, strlen(buf));
    }

    // Send message to owner:
    // Chat room <number> was closed.
    snprintf(buf, sizeof(buf), "Chat room %d was closed.\n", number);
    socket_util::Write(client_sockfd, buf, strlen(buf));

    // Close chat room
    chatroom_manager.close_chatroom(user_id, number);

    return false;
}

bool ChattingRoomCommandHandler::HandleCommand(int client_sockfd, const char *command) {
    std::string op;
    std::string message;

    std::stringstream ss(command);
    ss >> op;
    getline(ss, message);

    if (message.size() > 0 && message[0] == ' ') {
        message = message.substr(1);
    }

    // Remove ending '\n'
    if (message.size() > 0 && message[message.size() - 1] == '\n') {
        message = message.substr(0, message.size() - 1);
    }

    if ("/pin" == op || "/p" == op) {
        return HandlePinMessage_(client_sockfd, message); 
    } else if ("/delete-pin" == op || "/d" == op) {
        return HandleDeletePinMessage_(client_sockfd);
    } else if ("/exit-chat-room" == op || "/e" == op) {
        return HandleExitChatRoom_(client_sockfd);
    } else if ("/list-user" == op || "/l" == op) {
        return HandleListUser_(client_sockfd);
    } else if (op[0] == '/') {
        return HandleUnknownCommand_(client_sockfd);
    } else {
        message = std::string(command);
        return HandleChatMessage_(client_sockfd, message);
    }
}

bool ChattingRoomCommandHandler::HandlePinMessage_(int client_sockfd, std::string &message) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();
    
    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    int number = user_manager.get_chatroom_id(user_id);

    std::vector<int> chatroom_users;
    chatroom_manager.get_chatroom_users(number, chatroom_users);
    
    // Store message in chat room
    chatroom_manager.set_pin_message(number, username, message);

    // Send message to all users in the chat room:
    // Pin -> [<username>]: <message>\n
    for (size_t i = 0; i < chatroom_users.size(); ++i) {
        int user_id = chatroom_users[i];
        int sockfd = connection_manager.get_user_sockfd(user_id);
        snprintf(buf, sizeof(buf), "Pin -> [%s]: %s\n", username.c_str(), message.c_str());
        socket_util::Write(sockfd, buf, strlen(buf));
    }

    return false;
}

bool ChattingRoomCommandHandler::HandleDeletePinMessage_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    int number = user_manager.get_chatroom_id(user_id);
    
    // If there is no pin message in the chat room:
    // No pin message in chat room <number>\n
    std::string message;
    if (!chatroom_manager.get_pin_message(number, username, message)) {
        snprintf(buf, sizeof(buf), "No pin message in chat room %d\n", number);
        socket_util::Write(client_sockfd, buf, strlen(buf));
        return false;
    }

    chatroom_manager.set_pin_message(number, "", message);
    return false;
}

bool ChattingRoomCommandHandler::HandleExitChatRoom_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    int number = user_manager.get_chatroom_id(user_id);

    std::vector<int> chatroom_users;
    chatroom_manager.get_chatroom_users(number, chatroom_users);

    // Send message to other users:
    // <username> had left the chat room.
    for (size_t i = 0; i < chatroom_users.size(); ++i) {
        int user_id = chatroom_users[i];
        int sockfd = connection_manager.get_user_sockfd(user_id);
        if (sockfd != client_sockfd) {
            snprintf(buf, sizeof(buf), "%s had left the chat room.\n", username.c_str());
            socket_util::Write(sockfd, buf, strlen(buf));
        }
    }

    // Leave chat room
    connection_manager.leave_chatting_room(client_sockfd);
    chatroom_manager.leave_chatroom(user_id, number);
    user_manager.leave_chatroom(user_id, number);

    return false;
}

bool ChattingRoomCommandHandler::HandleListUser_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    int number = user_manager.get_chatroom_id(user_id);

    std::vector<int> chatroom_users;
    chatroom_manager.get_chatroom_users(number, chatroom_users);

    // For every user in the chat room:
    // <username> <status>
    // Note that the users are sorted by username

    std::sort(chatroom_users.begin(), chatroom_users.end(), [&](int a, int b) {
        return user_manager.get_username(a) < user_manager.get_username(b);
    });

    for (size_t i = 0; i < chatroom_users.size(); ++i) {
        int user_id = chatroom_users[i];
        std::string username = user_manager.get_username(user_id);
        UserStatus user_status = user_manager.get_user_status(user_id);
        std::string status;
        if (user_status == UserStatus::Online) {
            status = "online";
        } else if (user_status == UserStatus::Offline) {
            status = "offline";
        } else if (user_status == UserStatus::Busy) {
            status = "busy";
        }
        snprintf(buf, sizeof(buf), "%s %s\n", username.c_str(), status.c_str());
        socket_util::Write(client_sockfd, buf, strlen(buf));
    }

    return false;
}

bool ChattingRoomCommandHandler::HandleChatMessage_(int client_sockfd, std::string &message) {
    static char buf[MAX_COMMAND_LENGTH];
    static ConnectionManager& connection_manager = ConnectionManager::get_instance();
    static UserManager& user_manager = UserManager::get_instance();
    static ChatroomManager& chatroom_manager = ChatroomManager::get_instance();

    int user_id = connection_manager.get_user_id(client_sockfd);
    std::string username = user_manager.get_username(user_id);
    int number = user_manager.get_chatroom_id(user_id);

    std::vector<int> chatroom_users;
    chatroom_manager.get_chatroom_users(number, chatroom_users);

    // Store message in chat room
    chatroom_manager.send_message(user_id, number, message);

    // Send message to all users in the chat room:
    // [<username>]: <message>\n
    for (size_t i = 0; i < chatroom_users.size(); ++i) {
        int user_id = chatroom_users[i];
        int sockfd = connection_manager.get_user_sockfd(user_id);
        snprintf(buf, sizeof(buf), "[%s]: %s\n", username.c_str(), message.c_str());
        socket_util::Write(sockfd, buf, strlen(buf));
    }

    return false;
}

bool ChattingRoomCommandHandler::HandleUnknownCommand_(int client_sockfd) {
    static char buf[MAX_COMMAND_LENGTH];
    snprintf(buf, sizeof(buf), "Error: Unknown command\n");
    socket_util::Write(client_sockfd, buf, strlen(buf));
    return false;
}

} // namespace jhc_hw2