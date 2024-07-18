#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

enum Cell
{
    Empty,
    Unexplored,
    Explored,
    Wall,
    Exit
};

const pii directions[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
const char D[4] = {'W', 'S', 'A', 'D'};

const int MAX_MAZE_SIZE = 300;
const int X_ORIGIN = 150;
const int Y_ORIGIN = 150;
const int X_VIERPORT_SIZE = 7;
const int Y_VIERPORT_SIZE = 11;

void update_viewport(SocketStream &sock_stream, vector<vector<int>> &maze, int x, int y)
{
    string _;

    for (int i = -(X_VIERPORT_SIZE >> 1); i <= (X_VIERPORT_SIZE >> 1); ++i)
    {
        int ln;
        char _;
        sock_stream >> ln;
        sock_stream.get(_);
        sock_stream.get(_);

        for (int j = -(Y_VIERPORT_SIZE >> 1); j <= (Y_VIERPORT_SIZE >> 1); ++j)
        {
            sock_stream.get(_);
            if (maze[x + i][y + j] == Empty)
            {
                switch (_)
                {
                case '#':
                    maze[x + i][y + j] = Wall;
                    break;
                case '*':
                case '.':
                    maze[x + i][y + j] = Unexplored;
                    break;
                case 'E':
                    maze[x + i][y + j] = Exit;
                    break;
                default:
                    maze[x + i][y + j] = Empty;
                    break;
                }
            }
        }
    }

    sock_stream.getline(_);
    sock_stream.getline(_);
}

void send_steps(SocketStream &sock_stream, string &steps)
{
    string _;
    sock_stream << steps + '\n';
    sock_stream.getline(_);
    sock_stream.getline(_);
    steps = "";
}

int dfs(SocketStream &sock_stream, vector<vector<int>> &maze, int x, int y)
{
    static string steps = "";
    string _;

    if (maze[x][y] == Exit)
    {
        send_steps(sock_stream, steps);
        update_viewport(sock_stream, maze, x, y);
        return 0;
    }

    if (maze[x][y] == Explored)
        return -1;

    maze[x][y] = Explored;

    for (int i = 0; i < 4; ++i)
    {
        int nx = x + directions[i].first;
        int ny = y + directions[i].second;

        if (maze[nx][ny] == Empty)
        {
            send_steps(sock_stream, steps);
            update_viewport(sock_stream, maze, x, y);
        }

        if (maze[nx][ny] != Wall)
        {
            steps += D[i];

            int ret = dfs(sock_stream, maze, nx, ny);
            if (ret != -1)
                return ret;

            steps += D[i ^ 1];
        }
    }

    return -1;
}

int main()
{
    int sockfd = connect_to_server("140.113.213.213", 10304);
    if (sockfd == -1)
    {
        return 1;
    }

    SocketStream sock_stream(sockfd);
    vector<vector<int>> maze(MAX_MAZE_SIZE, vector<int>(MAX_MAZE_SIZE, 0));

    // Remove the first 8 lines
    std::string _;
    for (int i = 0; i < 8; ++i)
        sock_stream.getline(_);

    update_viewport(sock_stream, maze, X_ORIGIN, Y_ORIGIN);
    dfs(sock_stream, maze, X_ORIGIN, Y_ORIGIN);

    // Get Answer
    sock_stream.getline(_);
    cout << _ << endl;

    return 0;
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
    int writen_(const void *buf, size_t n);     // Function to ensure all n-byte are written
    int readline_(char *recvline, int MAXLINE); // Function to read a single line (‘\n’ inclusive)
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

int SocketStream::readline_(char *recvline, int MAXLINE)
{
    int n, rc;
    char c;

    for (n = 1; n < MAXLINE; n++)
    {
        if ((rc = read(sockfd_, &c, 1)) == 1)
        {
            *recvline++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            if (n == 1)
                return 0;
            else
                break;
        }
        else
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
    }

    *recvline = 0;
    return n;
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
    // std::cerr << "size: " << size << std::endl;

    if (size < 2)
    {
        int bytes_received = readline_(buf_, BUF_SIZE);
        if (bytes_received == -1)
        {
            std::cerr << "Failed to receive data from server\n";
            exit(1);
        }

        // std::cerr << "Received: " << std::string(buf_, bytes_received);

        iss_ << std::string(buf_, bytes_received);
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