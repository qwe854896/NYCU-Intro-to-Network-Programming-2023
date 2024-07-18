#ifndef JHC_H
#define JHC_H
#include JHC_H __FILE__ JHC_H

/* Constants */

// limit 1000 delay 100ms 50ms loss 40% corrupt 10% duplicate 5% rate 10Mbit
constexpr const uint32_t MAX_FILE_SIZE = 36000000;
constexpr const uint16_t MAX_NUM_FILES = 1000;
constexpr const uint16_t LIMIT = 1000;

constexpr const uint32_t TIME_UNIT = 1000;                           // in microseconds
// constexpr const uint32_t NUM_CHECK_FILE = 200;
// constexpr const uint32_t SRV_RECV_TIMEOUT = 0.840 * TIME_UNIT;
// constexpr const uint32_t CLT_RECV_TIMEOUT = 0.750 * NUM_CHECK_FILE * TIME_UNIT;
constexpr const uint32_t SRV_SEND_TIMEOUT = 50 * TIME_UNIT;
constexpr const uint32_t CLT_SEND_TIMEOUT = 0.420 * TIME_UNIT;

constexpr const uint32_t MAX_HEADER_SIZE = 4 * MAX_NUM_FILES;
constexpr const uint16_t MTU = LIMIT - 28;                        // 28 bytes for IP + UDP header
constexpr const uint16_t ACK_BIT = (MTU - 1) << 3;                       // 8 bits per byte
constexpr const uint16_t PKT_SIZE = MTU - 2;                      // 2 bytes for indexing packets
constexpr const uint16_t MAX_PACKETS = (MAX_HEADER_SIZE + MAX_FILE_SIZE + PKT_SIZE - 1) / PKT_SIZE;
constexpr const uint16_t SPECIAL_PACKET_SIZE = (MAX_HEADER_SIZE + PKT_SIZE - 1) / PKT_SIZE;
constexpr const char *SPECIAL_HEADER = "0";

/* Global Variables*/

extern uint8_t header[MAX_HEADER_SIZE];
extern std::string filename[MAX_NUM_FILES + 1];

/* Struct */

// Total size of the header is 4 * MAX_NUM_FILES
// Which is 4000 bytes
struct Header {
    uint32_t size[MAX_NUM_FILES]; // 4 bytes
} __attribute__((packed));

// Total size of the packet is 2 + PKT_SIZE
// Which is 972 bytes
struct Packet {
    uint16_t index; // 2 bytes
    uint8_t data[PKT_SIZE];
} __attribute__((packed));

// Total size of the metadata is 10 bytes
struct PacketMetadata {
    uint16_t index;             // 2 bytes
    uint16_t size;              // 2 bytes, initially 0
    uint16_t start_file_index;  // 2 bytes
    uint32_t start_file_offset; // 4 bytes
} __attribute__((packed));

// Use bitset to store the status of each packet
// Which is 972 bytes
struct Ack {
    uint8_t checksum;
    uint8_t content[MTU - 1];
} __attribute__((packed));

// Won't be used
struct Protocol {
    uint32_t size[MAX_NUM_FILES]; // 4 bytes
    uint8_t file[];
} __attribute__((packed));

/* Functions */

std::string index2string(int index);
void file_length2packet_metadata(const std::vector<uint32_t> &file_length, std::vector<PacketMetadata> &packet_metadata);
void load_file(const std::string &file_name, uint32_t offset, uint32_t size, uint8_t *data);
void write_file(const std::string &file_name, uint32_t offset, uint32_t size, const uint8_t *data);
void load_files2buffer_by_metadata(const std::vector<uint32_t> &file_length, const PacketMetadata &packet_metadata, Packet *buffer);
void write_buffer2files_by_metadata(const std::vector<uint32_t> &file_length, const PacketMetadata &packet_metadata, const Packet *buffer);
void ensure_queued_packets_are_sent(int s, const sockaddr_in& sin, const std::vector<PacketMetadata>& packet_metadata, const std::vector<uint32_t>& file_length, std::queue<size_t> &queued_packets_index);
void send_ack_for_queued_packets(int s, const sockaddr_in& sin, const std::vector<PacketMetadata>& packet_metadata, const std::vector<uint32_t>& file_length, size_t start, size_t offset, Packet *last_packet = nullptr);

#else

#include <sys/sendfile.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <bitset>
#include <chrono>
#include <thread>
#include <vector>
#include <queue>

int Socket(int __domain, int __type, int __protocol);
int Setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);
int Bind(int __fd, const sockaddr *__addr, socklen_t __len);
int Connect(int __fd, const sockaddr *__addr, socklen_t __len);
int Listen(int __fd, int __n);
int Accept(int __fd, sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len);
ssize_t Recv(int __fd, void *__buf, size_t __n, int __flags);
ssize_t Send(int __fd, const void *__buf, size_t __n, int __flags);
ssize_t Sendto(int __fd, const void *__buf, size_t __n, int __flags, const struct sockaddr *__addr, socklen_t __addr_len);
ssize_t Recvfrom(int __fd, void* __buf, size_t __n, int __flags, struct sockaddr* __addr, socklen_t* __addr_len);

#endif
