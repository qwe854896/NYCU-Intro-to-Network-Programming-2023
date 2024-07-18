#pragma once

#include <sys/socket.h>

namespace jhc_hw2 {
namespace socket_util {

int Socket(int __domain, int __type, int __protocol);
void Bind(int __fd, const struct sockaddr *__addr, socklen_t __len);
void Listen(int __fd, int __n);
int Accept(int __fd, struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len);
void Connect(int __fd, const struct sockaddr *__addr, socklen_t __len);
ssize_t Read(int __fd, void *__buf, size_t __nbytes);
ssize_t Write(int __fd, const void *__buf, size_t __n);
ssize_t ReadLine(int __fd, void *__buf, size_t __nbytes);
int Setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);

} // namespace socket_util
} // namespace jhc_hw2