#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

char buffer[BUFFER_SIZE];
char buffer2[BUFFER_SIZE];

int main() {
    int clientSocket, n;
    struct sockaddr_in serverAddress;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = RCV_TIMEOUT;

    // Create UDP socket
    clientSocket = Socket(AF_INET, SOCK_DGRAM, 0);
    Setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Set up server address struct
    memset((char *)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddress.sin_port = htons(PORT);

    /* Latency */
    // Send a dummy message to the server to measure latency
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; ++i) {
        sendto(clientSocket, "A", 1, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / TEST_ITERATIONS;

    sendto(clientSocket, "B", 1, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    recvfrom(clientSocket, buffer2, BUFFER_SIZE, 0, nullptr, nullptr);

    /* BANDWIDTH */
    long long total_bytes = 0;

    buffer[0] = 'C';
    while (buffer[0] == 'C') {
        n = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now() + std::chrono::seconds(TEST_DURATION_SECONDS);
    while (std::chrono::high_resolution_clock::now() < end) {
        n = recvfrom(clientSocket, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        total_bytes += n;
    }
    sleep(TEST_DURATION_SECONDS);

    long long total_times = TEST_DURATION_SECONDS;

    long long total_bits = total_bytes << 3ll;
    total_times = total_times * 1000000ll;

    long double bandwidth;

    /* Round End */
    latency = (latency + 1000) / 2000;                        // Round to nearest ms
    bandwidth = (total_bits + total_times / 2) / total_times; // Round to nearest Mbps (2 decimal places

    std::cout << "# RESULTS: delay = " << latency << " ms, bandwidth = " << bandwidth << " Mbps\n";

    close(clientSocket);

    return 0;
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <sys/time.h>
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

#endif