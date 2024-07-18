#include "socket_util.h"

#include <unistd.h>
#include <iostream>

#define perror(msg) \
    do { \
        std::cerr << msg << std::endl; \
        exit(EXIT_FAILURE); \
    } while (0)

namespace jhc_hw2 {
namespace socket_util {

int Socket(int __domain, int __type, int __protocol) {
    int sockfd = socket(__domain, __type, __protocol);
    if (sockfd == -1) {
        perror("socket");
    }
    return sockfd;
}

void Bind(int __fd, const struct sockaddr *__addr, socklen_t __len) {
    if (bind(__fd, __addr, __len) == -1) {
        perror("bind");
    }
}

void Listen(int __fd, int __n) {
    if (listen(__fd, __n) == -1) {
        perror("listen");
    }
}

int Accept(int __fd, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len) {
    int connfd = accept(__fd, __addr, __addr_len);
    if (connfd == -1) {
        perror("accept");
    }
    return connfd;
}

void Connect(int __fd, const struct sockaddr *__addr, socklen_t __len) {
    if (connect(__fd, __addr, __len) == -1) {
        perror("connect");
    }
}

ssize_t Read(int __fd, void *__buf, size_t __nbytes) {
    ssize_t n = read(__fd, __buf, __nbytes);
    if (n == -1) {
        perror("read");
    }
    return n;
}

// Read until '\n' or '\0'
// Replace '\n' with '\0'
ssize_t ReadLine(int __fd, void *__buf, size_t __nbytes) {
    size_t n = 0;
    char c;
    while (n < __nbytes) {
        if (read(__fd, &c, 1) == -1) {
            perror("read");
        }
        ((char *)__buf)[n++] = c;
        if (c == '\n' || c == '\0') {
            break;
        }
    }
    ((char *)__buf)[n - 1] = '\0';
    return n;
}

ssize_t Write(int __fd, const void *__buf, size_t __n) {
    ssize_t n = write(__fd, __buf, __n);

    // Handle SIGPIPE
    if (n == -1 && errno == EPIPE) {
        return 0;
    }

    if (n == -1) {
        perror("write");
    }
    return n;
}

int Setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) {
    int ret = setsockopt(__fd, __level, __optname, __optval, __optlen);
    if (ret == -1) {
        perror("setsockopt");
    }
    return ret;
}

} // namespace socket_util
} // namespace jhc_hw2