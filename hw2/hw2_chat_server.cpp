#include "chat_server.h"

#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [port number]" << std::endl;
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    std::cerr << "Server is listening on port " << port << std::endl;

    jhc_hw2::ChatServer chat_server;
    chat_server.Start(port);
}