#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

const string STUDENT_ID = "109550157";

string get(const char *path, SocketStream &ss, const string &SERVER, const string &PORT)
{
    ss << "GET " << path << " HTTP/1.1\r\n"
       << "Connection: keep-alive\r\n"
       << "Content-Length: 0\r\n"
       << "Host: " << SERVER << ":" << PORT << "\r\n\r\n";

    string _, result;
    while ((ss.getline(_), _).size() > 1)
        ;
    ss.getline(result);

    return result;
}

string post(const char *path, const string &data, SocketStream &ss, const string &SERVER, const string &PORT)
{
    string body = "------WebKitFormBoundary1xWn0uFhnAt1gcEY\r\n"
                  "Content-Disposition: form-data; name=\"file\"; filename=\"otp.txt\"\r\n"
                  "Content-Type: application/octet-stream\r\n\r\n" +
                  data + "\r\n"
                         "------WebKitFormBoundary1xWn0uFhnAt1gcEY--\r\n\r\n";

    ss << "POST " << path << " HTTP/1.1\r\n"
       << "Connection: keep-alive\r\n"
       << "Content-Length: " << body.length() << "\r\n"
       << "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary1xWn0uFhnAt1gcEY\r\n"
       << "Host: " << SERVER << ":" << PORT << "\r\n\r\n"
       << body;

    string _, result;
    while ((ss.getline(_), _).size() > 1)
        ;
    ss.getline(result);

    return result;
}

int main()
{
    string SERVER = "140.113.213.213";
    string PORT = "10314";
    int sockfd = connect_to_server(SERVER.c_str(), stoi(PORT));

    SocketStream ss(sockfd);

    string server_addr = get("/addr", ss, SERVER, PORT);

    close(sockfd);

    SERVER = server_addr.substr(0, server_addr.find(":"));
    PORT = server_addr.substr(server_addr.find(":") + 1);

    sockfd = connect_to_server(SERVER.c_str(), stoi(PORT));
    ss.set_sockfd(sockfd);

    string otp = get(("/otp?name=" + STUDENT_ID).c_str(), ss, SERVER, PORT);
    cout << "OTP: " << otp << endl;
    close(sockfd);

    sockfd = connect_to_server(SERVER.c_str(), stoi(PORT));
    ss.set_sockfd(sockfd);

    string upload_result = post("/upload", otp, ss, SERVER, PORT);

    cout << "Upload result: " << upload_result << endl;

    sockfd = connect_to_server(SERVER.c_str(), stoi(PORT));
    ss.set_sockfd(sockfd);

    string logs = get("/logs", ss, SERVER, PORT);
    cout << logs << endl;
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include <unistd.h>

#include <arpa/inet.h>

#include <iostream>

#include <sstream>

#include <algorithm>

using namespace std;
using pii = pair<int, int>;

const size_t BUF_SIZE = 65536;

class SocketStream
{
private:
    int writen_(const void *buf, size_t n); // Function to ensure all n-byte are written
    int read_(char *recvline);              // Function to read a single line (‘\n’ inclusive)
    void ensure_buffer_full_();

public:
    SocketStream(int sockfd) : sockfd_(sockfd) {}

    void close() { ::close(sockfd_); }
    void set_sockfd(int sockfd)
    {
        sockfd_ = sockfd;
        iss_.clear();
        iss_.str("");
        oss_.clear();
        oss_.str("");
    }

    template <typename T>
    SocketStream &operator>>(T &obj);

    template <typename T>
    SocketStream &operator<<(const T &obj);

    void get(char &c);
    void getline(std::string &str);

private:
    int sockfd_;
    char buf_[BUF_SIZE];
    std::stringstream iss_;
    std::stringstream oss_;
};

int SocketStream::read_(char *recvline)
{
    int n = 0, rc;

    while (true)
    {
        rc = read(sockfd_, recvline, BUF_SIZE);

        if (rc == -1)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (rc == 0)
        {
            return n;
        }

        n += rc;
        recvline += rc - 1;

        if (*recvline == '\n')
        {
            recvline++;
            *recvline = 0;
            return n;
        }
        else
            recvline++;
    }
}

int SocketStream::writen_(const void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    const char *ptr = (const char *)buf;

    while (nleft > 0)
    {
        if ((nwritten = write(sockfd_, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

template <typename T>
SocketStream &SocketStream::operator>>(T &obj)
{
    ensure_buffer_full_();
    iss_ >> obj;
    return *this;
}

template <typename T>
SocketStream &SocketStream::operator<<(const T &obj)
{
    oss_.str("");
    oss_ << obj;

    std::string str = oss_.str();

    int bytes_sent = writen_(str.c_str(), str.size());
    if (bytes_sent == -1)
    {
        std::cerr << "Failed to send data to server\n";
        exit(1);
    }

    return *this;
}

void SocketStream::ensure_buffer_full_()
{
    int size = (int)iss_.tellp() - (int)iss_.tellg();

    if (size < 2)
    {
        int bytes_received = read_(buf_);
        if (bytes_received == -1)
        {
            std::cerr << "Failed to receive data from server\n";
            exit(1);
        }

        // std::cerr << "Received: " << std::string(buf_, bytes_received) << "Received end\n";

        iss_ << buf_;
    }
}

void SocketStream::get(char &c)
{
    ensure_buffer_full_();
    iss_.get(c);
}

void SocketStream::getline(std::string &str)
{
    ensure_buffer_full_();
    std::getline(iss_, str);
}

int connect_to_server(const char *server_ip, uint16_t server_port)
{
    // Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    // Connect to the server
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);
    if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to connect to server\n";
        return -1;
    }

    return sock;
}

#endif