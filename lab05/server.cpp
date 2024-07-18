#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

char buffer[BUFFER_SIZE];
char ALLC[BUFFER_SIZE];
char ALLD[BUFFER_SIZE];

int main() {
    int serverSocket, length, n;
    struct sockaddr_in serverAddress, clientAddress;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = RCV_TIMEOUT;

    // Create UDP socket
    serverSocket = Socket(AF_INET, SOCK_DGRAM, 0);
    Setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Set up server address struct
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddress.sin_port = htons(PORT);

    // Bind the socket
    Bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    length = sizeof(clientAddress);
    strcpy(ALLC, std::string(BUFFER_SIZE, 'C').c_str());
    strcpy(ALLD, std::string(BUFFER_SIZE, 'D').c_str());

    while (true) {
        buffer[0] = 'A';
        while (buffer[0] == 'A') {
            n = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddress, (socklen_t *)&length);
            n = sendto(serverSocket, buffer, n, 0, (struct sockaddr *)&clientAddress, length);
        }

        for (int i = 0; i < TEST_ITERATIONS; ++i) {
            sendto(serverSocket, ALLC, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddress, length);
        }

        auto end = std::chrono::high_resolution_clock::now() + std::chrono::seconds(TEST_DURATION_SECONDS << 1);
        while (std::chrono::high_resolution_clock::now() < end) {
            sendto(serverSocket, ALLD, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddress, length);
        }
    }

    close(serverSocket);
    return 0;
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include "common.h"

int Socket(int __domain, int __type, int __protocol) {
    int fd = socket(__domain, __type, __protocol);
    if (fd == -1) {
        perror("Socket creation error");
        exit(1);
    }
    return fd;
}

int Setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) {
    int status = setsockopt(__fd, __level, __optname, __optval, __optlen);
    if (status == -1) {
        perror("Setsockopt error");
        exit(1);
    }
    return status;
}

int Bind(int __fd, const sockaddr *__addr, socklen_t __len) {
    int status = bind(__fd, __addr, __len);
    if (status == -1) {
        perror("Binding error");
        exit(1);
    }
    return status;
}

#endif