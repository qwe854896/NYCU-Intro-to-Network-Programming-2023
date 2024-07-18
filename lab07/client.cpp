#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

const int N = 30;

bool isSafe(int board[N][N], int row, int col)
{
    int i, j;
 
    for (i = 0; i < N; i++)
        if (board[row][i])
            return false;
 
    for (i = row, j = col; i >= 0 && j >= 0; i--, j--)
        if (board[i][j])
            return false;
    
    for (i = row, j = col; i < N && j < N; i++, j++)
        if (board[i][j])
            return false;
 
    for (i = row, j = col; j >= 0 && i < N; i++, j--)
        if (board[i][j])
            return false;
    
    for (i = row, j = col; j < N && i >= 0; i--, j++)
        if (board[i][j])
            return false;
 
    return true;
}

bool solveNQUtil(int board[N][N], int col)
{
    if (col >= N)
        return true;

    for (int i = 0; i < N; ++i) {
        if (board[i][col] == 2)
            return solveNQUtil(board, col + 1);
    }
 
    for (int i = 0; i < N; i++) {
        if (isSafe(board, i, col)) {
            board[i][col] = 1;
 
            if (solveNQUtil(board, col + 1))
                return true;
 
            board[i][col] = 0;
        }
    }
 
    return false;
}

int main()
{
    int sockfd = connect_to_server("/queen.sock");
    if (sockfd == -1)
    {
        return 1;
    }

    SocketStream sock_stream(sockfd);

    std::string board_str;

    sock_stream << "S\n";
    sock_stream.getline(board_str);

    board_str = board_str.substr(4, N * N);

    int board[N][N];
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; j++) {
            if (board_str[i * N + j] == 'Q')
                board[i][j] = 2;
            else
                board[i][j] = 0;
            
        }
    }

    if (solveNQUtil(board, 0) == false) {
        std::cout << "No solution" << endl;
        return 0;
    }

    std::string _;

    std::cout << "Final Board: \n";
    for (int i = 0; i < N; ++i) {
        string str = "";
        for (int j = 0; j < N; j++) {
            if (board[i][j]) {
                str += "Q";
            }
            else
                str += ".";
        }
        std::cout << str << endl;
    }
    std::cout << "Final Board end\n";

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; j++) {
            if (board[i][j] == 1) {
                std::cout << "M " << i << " " << j << endl;
                _ = "M " + to_string(i) + " " + to_string(j) + "\n";
                sock_stream << _;

                sock_stream.getline(_);
                std::cout << _ << endl;
            }
        }
    }

    sock_stream << "C\n";
    sock_stream.getline(_);

    std::cout << _ << endl;

    return 0;
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include <unistd.h>

#include <arpa/inet.h>

#include <iostream>

#include <sstream>

#include <algorithm>

#include <sys/un.h>

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
            return n;

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

int connect_to_server(const char *server_ip)
{
    // Create a UNIX STREAM socket
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    // Connect to the server
    sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, server_ip);
    if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to connect to server\n";
        return -1;
    }

    return sock;
}

#endif