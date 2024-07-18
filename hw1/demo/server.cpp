#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

std::string url_decode(const std::string &encoded) {
    std::ostringstream decoded;
    for (std::size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%') {
            std::string hex = encoded.substr(i + 1, 2);
            int value;
            std::istringstream(hex) >> std::hex >> value;
            decoded << static_cast<char>(value);
            i += 2;
        } else if (encoded[i] == '+') {
            decoded << ' ';
        } else {
            decoded << encoded[i];
        }
    }
    return decoded.str();
}

std::string get_mime_type(const std::string &file_path) {
    std::string extension = file_path.substr(file_path.find_last_of(".") + 1);
    if (extension == "html") {
        return "text/html";
    } else if (extension == "css") {
        return "text/css";
    } else if (extension == "js") {
        return "application/javascript";
    } else if (extension == "png") {
        return "image/png";
    } else if (extension == "jpg" || extension == "jpeg") {
        return "image/jpeg";
    } else if (extension == "gif") {
        return "image/gif";
    } else if (extension == "mp3") {
        return "audio/mpeg";
    } else if (extension == "wav") {
        return "audio/wav";
    } else {
        return "application/octet-stream";
    }
}

std::string get_file_contents(const std::string &file_path) {
    std::ifstream file(file_path);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

void send_response(int client_socket, const std::string &status_line, const std::string &headers, const std::string &body, bool is_https=false, SSL *cSSL=nullptr) {
    std::string response = status_line + "\r\n" + headers + "\r\n" + body + "\r\n";
    if (is_https) {
        SSL_write(cSSL, response.c_str(), response.size());
    } else {
        Write(client_socket, response.c_str(), response.size());
    }
}

void http_handler(int client_socket, const char *request) {
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    if (method != "GET") {
        send_response(client_socket, "HTTP/1.0 501 Not Implemented", "", "");
        return;
    }

    path = url_decode(path);

    // Drop query parameters
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos - 1);
    }

    if (path.empty()) {
        path = "/";
    }

    std::string file_path = DOCUMENT_ROOT + path;
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) == -1) {
        send_response(client_socket, "HTTP/1.0 404 Not Found", "", "");
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        if (file_path.back() != '/') {
            send_response(client_socket, "HTTP/1.0 301 Moved Permanently", "Location: " + path + "/", "");
            return;
        }
        file_path += "index.html";
        if (stat(file_path.c_str(), &file_stat) == -1) {
            send_response(client_socket, "HTTP/1.0 403 Forbidden", "", "");
            return;
        }
    }

    std::string response = "HTTP/1.0 200 OK\r\nContent-Type: " + get_mime_type(file_path) + "\r\nContent-Length: " + std::to_string(file_stat.st_size) + "\r\n\r\n";

    send(client_socket, response.c_str(), response.size(), 0);
    sendfile(client_socket, open(file_path.c_str(), O_RDONLY), nullptr, file_stat.st_size);
}

void https_handler(int client_socket, const char *request, SSL *cSSL) {
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    if (method != "GET") {
        send_response(client_socket, "HTTP/1.0 501 Not Implemented", "", "", true, cSSL);
        return;
    }

    // Drop query parameters
    size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        path = path.substr(0, query_pos);
    }

    std::string file_path = DOCUMENT_ROOT + url_decode(path);
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) == -1) {
        send_response(client_socket, "HTTP/1.0 404 Not Found", "", "", true, cSSL);
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        if (file_path.back() != '/') {
            send_response(client_socket, "HTTP/1.0 301 Moved Permanently", "Location: " + path + "/", "", true, cSSL);
            return;
        }
        file_path += "index.html";
        if (stat(file_path.c_str(), &file_stat) == -1) {
            send_response(client_socket, "HTTP/1.0 403 Forbidden", "", "", true, cSSL);
            return;
        }
    }

    std::string response = "HTTP/1.0 200 OK\r\nContent-Type: " + get_mime_type(file_path) + "\r\nContent-Length: " + std::to_string(file_stat.st_size) + "\r\n\r\n";

    SSL_write(cSSL, response.c_str(), response.size());
    SSL_write(cSSL, get_file_contents(file_path).c_str(), file_stat.st_size);
}

void http_worker(int http_server_socket) {
    while (true) {
        char buffer[BUFFER_SIZE];
        sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);

        int client_socket = accept(http_server_socket, (sockaddr *)&client_address, &client_address_size);

        recv(client_socket, buffer, BUFFER_SIZE, 0);
        http_handler(client_socket, buffer);
        close(client_socket);
    }
}

void https_worker(int https_server_socket, SSL_CTX *sslctx) {
    char buffer[BUFFER_SIZE];
    while (true) {
        sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = Accept(https_server_socket, (sockaddr *)&client_address, &client_address_size);

        SSL *cSSL = SSL_new(sslctx);
        SSL_set_fd(cSSL, client_socket);
        SSL_accept(cSSL);

        SSL_read(cSSL, buffer, BUFFER_SIZE);
        // handle_request(client_socket, buffer, true, cSSL);
        https_handler(client_socket, buffer, cSSL);

        SSL_shutdown(cSSL);
        SSL_free(cSSL);
        close(client_socket);
    }
}

int main(int argc, char *argv[]) {
    /* Arguments */
    DOCUMENT_ROOT = argv[2];
    int port = std::stoi(argv[1]);
    int https_port = std::stoi(argv[3]);
    std::string cert_file = argv[4];

    /* HTTP */
    int http_server_socket;
    init_socket(http_server_socket, port);

    /* HTTPS */
    int https_server_socket;
    init_socket(https_server_socket, https_port);

    SSL_CTX *sslctx = create_context();
    SSL_CTX_use_certificate_file(sslctx, cert_file.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(sslctx, cert_file.c_str(), SSL_FILETYPE_PEM);

    /* Workers */
    std::vector<std::thread> http_workers(NUM_HTTP_THREADS);
    std::vector<std::thread> https_workers(NUM_HTTPS_THREADS);
    for (int i = 0; i < NUM_HTTP_THREADS; ++i) {
        http_workers[i] = std::thread(http_worker, http_server_socket);
    }
    for (int i = 0; i < NUM_HTTPS_THREADS; ++i) {
        https_workers[i] = std::thread(https_worker, https_server_socket, sslctx);
    }
    for (int i = 0; i < NUM_HTTP_THREADS; ++i) {
        http_workers[i].join();
    }
    for (int i = 0; i < NUM_HTTPS_THREADS; ++i) {
        https_workers[i].join();
    }

    /* Close */
    close(http_server_socket);
    close(https_server_socket);
    SSL_CTX_free(sslctx);
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

const int NUM_HTTP_THREADS = 64;
const int NUM_HTTPS_THREADS = 1;
const int BUFFER_SIZE = 1024;

std::string DOCUMENT_ROOT;

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

int Listen(int __fd, int __n) {
    int status = listen(__fd, __n);
    if (status == -1) {
        perror("Listening error");
        exit(1);
    }
    return status;
}

int Accept(int __fd, sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len) {
    int fd = accept(__fd, __addr, __addr_len);
    if (fd == -1) {
        perror("Accepting error");
        exit(1);
    }
    return fd;
}

ssize_t Recv(int __fd, void *__buf, size_t __n, int __flags) {
    ssize_t bytes_received = recv(__fd, __buf, __n, __flags);
    if (bytes_received == -1) {
        perror("Receiving error");
        close(__fd);
        return -1;
    }
    return bytes_received;
}

ssize_t Send(int __fd, const void *__buf, size_t __n, int __flags) {
    size_t nleft = __n;
    ssize_t nwritten;
    const char *ptr = (const char *)__buf;

    while (nleft > 0) {
        if ((nwritten = send(__fd, ptr, nleft, __flags)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                continue;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return __n;
}

ssize_t Read(int __fd, void *__buf, size_t __nbytes) {
    ssize_t bytes_read = read(__fd, __buf, __nbytes);
    if (bytes_read == -1) {
        perror("Reading error");
        close(__fd);
        return -1;
    }
    return bytes_read;
}

ssize_t Write(int __fd, const void *__buf, size_t __n) {
    size_t nleft = __n;
    ssize_t nwritten;
    const char *ptr = (const char *)__buf;

    while (nleft > 0) {
        if ((nwritten = write(__fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                continue;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return __n;
}

void init_socket(int &server_socket, int port) {
    server_socket = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    Bind(server_socket, (sockaddr *)&server_address, sizeof(server_address));
    Listen(server_socket, 2400);
}

SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

#endif