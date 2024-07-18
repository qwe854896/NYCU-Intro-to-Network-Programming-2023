#include "common.h"

uint8_t header[MAX_HEADER_SIZE];
std::string filename[MAX_NUM_FILES + 1];

std::string index2string(int index) {
    std::string file_name = std::to_string(index);
    while (file_name.size() < 6) {
        file_name = "0" + file_name;
    }
    return file_name;
}

void file_length2packet_metadata(const std::vector<uint32_t>& file_length, std::vector<PacketMetadata>& packet_metadata) {
    // All packet metadata should be filled with full PKT_SIZE
    // If one file cannot fill a packet, then next file will fill the rest of the packet

    packet_metadata.clear();
    packet_metadata.emplace_back();
    memset(&packet_metadata.back(), 0, sizeof(PacketMetadata));

    for (size_t i = 0; i < file_length.size(); ++i) {
        uint32_t file_size = file_length[i];
        uint32_t file_offset = 0;

        while (file_size > 0) {
            if (packet_metadata.back().size == PKT_SIZE) {
                packet_metadata.emplace_back(PacketMetadata());
                memset(&packet_metadata.back(), 0, sizeof(PacketMetadata));

                packet_metadata.back().size = 0;
                packet_metadata.back().start_file_index = i;
                packet_metadata.back().start_file_offset = file_offset;
            }

            uint32_t size = std::min(file_size, (uint32_t)PKT_SIZE - packet_metadata.back().size);

            packet_metadata.back().size += size;

            file_size -= size;
            file_offset += size;
        }
    }
}

void load_file(const std::string& file_name, uint32_t offset, uint32_t size, uint8_t* data) {
    if (file_name == SPECIAL_HEADER) {
        memcpy(data, header + offset, size);
        return;
    }

    int fd = open(file_name.c_str(), O_RDONLY);
    lseek(fd, offset, SEEK_SET);
    read(fd, data, size);
    close(fd);
}

void write_file(const std::string& file_name, uint32_t offset, uint32_t size, const uint8_t* data) {
    if (file_name == SPECIAL_HEADER) {
        memcpy(header + offset, data, size);
        return;
    }

    int fd = open(file_name.c_str(), O_WRONLY);
    lseek(fd, offset, SEEK_SET);
    write(fd, data, size);
    close(fd);
}

void load_files2buffer_by_metadata(const std::vector<uint32_t>& file_length, const PacketMetadata& packet_metadata, Packet* buffer) {
    memset(buffer, 0, sizeof(Packet));

    uint16_t remain = packet_metadata.size;
    uint16_t file_index = packet_metadata.start_file_index;
    uint32_t file_offset = packet_metadata.start_file_offset;

    buffer->index = packet_metadata.index;
    uint8_t* data = buffer->data;

    while (remain > 0) {
        uint32_t file_size = file_length[file_index];
        uint32_t read_size = std::min(file_size - file_offset, (uint32_t)remain);

        load_file(filename[file_index], file_offset, read_size, data);

        data += read_size;
        remain -= read_size;
        if (remain > 0) {
            ++file_index;
            file_offset = 0;
        }
    }
}

void write_buffer2files_by_metadata(const std::vector<uint32_t>& file_length, const PacketMetadata& packet_metadata, const Packet* buffer) {
    uint16_t remain = packet_metadata.size;
    uint16_t file_index = packet_metadata.start_file_index;
    uint32_t file_offset = packet_metadata.start_file_offset;

    const uint8_t* data = buffer->data;

    while (remain > 0) {
        uint32_t file_size = file_length[file_index];
        uint32_t write_size = std::min(file_size - file_offset, (uint32_t)remain);

        write_file(filename[file_index], file_offset, write_size, data);

        data += write_size;
        remain -= write_size;
        if (remain > 0) {
            ++file_index;
            file_offset = 0;
        }
    }
}

void recv_ack(int s, std::bitset<ACK_BIT>& completed, size_t offset, size_t size) {
    sockaddr_in csin;
    socklen_t csinlen = sizeof(csin);

    char ack_buf[MTU] = {0};
    Ack* ack = (Ack*)ack_buf;
    ack->checksum = 0xFF;

    for (;;) {
        // ssize_t recv_bytes = recvfrom(s, ack, MTU, 0, (struct sockaddr*)&csin, &csinlen);

        // if (recv_bytes <= 0) {
        //     continue;
        // }

        recvfrom(s, ack, MTU, 0, (struct sockaddr*)&csin, &csinlen);

        if (ack->checksum != (offset & 0xFF)) {
            continue;
        }

        for (size_t i = 0; i < MTU - 1; ++i) {
            for (size_t j = 0; j < 8; ++j) {
                if (ack->content[i] & (1 << j)) {
                    completed[i << 3 | j] = 1;
                }
            }
        } 

        if (completed.count() == size) {
            break;
        }
    }
}

// The size of queued_packets_index must not exceed ACK_BIT
// The index in queued_packets_index should be consecutive
void ensure_queued_packets_are_sent(int s, const sockaddr_in& sin, const std::vector<PacketMetadata>& packet_metadata, const std::vector<uint32_t>& file_length, std::queue<size_t> &queued_packets_index) {
    std::bitset<ACK_BIT> completed{0};

    // Because the index in queued_packets_index is consecutive
    // We can use the first index as the offset
    size_t offset = queued_packets_index.front();
    size_t size = queued_packets_index.size();

    std::thread recv_ack_thread(recv_ack, s, std::ref(completed), offset, size);

    socklen_t sinlen = sizeof(sin);

    char buf[MTU] = {0};
    Packet* packet = (Packet*)buf;

    while (!queued_packets_index.empty()) {
        // Get the next packet to send
        size_t i = queued_packets_index.front();
        queued_packets_index.pop();

        // Check if the packet is already received
        if (completed[i - offset]) {
            continue;
        }
        queued_packets_index.push(i);

        // Send Packets
        load_files2buffer_by_metadata(file_length, packet_metadata[i], packet);
        sendto(s, buf, MTU, 0, (struct sockaddr*)&sin, sinlen);

        // Sleep
        std::this_thread::sleep_for(std::chrono::microseconds(CLT_SEND_TIMEOUT));
    }

    std::cout << "All packets are sent " << offset << std::endl;
}

void send_ack_for_queued_packets(int s, const sockaddr_in& sin, const std::vector<PacketMetadata>& packet_metadata, const std::vector<uint32_t>& file_length, size_t offset, size_t size, Packet* last_packet) {
    std::bitset<ACK_BIT> completed{0};

    char buf[MTU] = {0};
    Packet* packet = (Packet*)buf;

    char ack_buf[MTU] = {0};
    Ack* ack = (Ack*)ack_buf;
    ack->checksum = offset & 0xFF;

    sockaddr_in csin;
    socklen_t csinlen = sizeof(csin);

    size_t count = size;
    size_t end = offset + size;

    // chrono high resolution clock
    auto start = std::chrono::high_resolution_clock::now();

    for (;;) {
        // Received Packets
        // ssize_t recv_byte = recvfrom(s, packet, sizeof(Packet), 0, (struct sockaddr*)&csin, &csinlen);

        // if (recv_byte <= 0) {
        //     continue;
        // }

        recvfrom(s, packet, 0, MTU, (struct sockaddr*)&csin, &csinlen);

        if (start + std::chrono::milliseconds(SRV_SEND_TIMEOUT) < std::chrono::high_resolution_clock::now()) {
            sendto(s, ack, MTU, 0, (struct sockaddr*)&csin, csinlen);
            start = std::chrono::high_resolution_clock::now();
        }

        // std::cout << "Receive packet " << packet->index << std::endl;

        if (packet->index < offset || packet->index >= end) {
            continue;
        }

        size_t index = packet->index - offset;

        // Check if the packet is already received
        if (completed[index]) {
            continue;
        }

        std::cout << "Receive packet " << packet->index << std::endl;

        // Update Status
        completed.set(index);
        ack->content[index >> 3] |= 1 << (index & 7);

        // Write Packets to file
        write_buffer2files_by_metadata(file_length, packet_metadata[packet->index], packet);

        // Special Case: Last Packet
        if (last_packet && packet->index == last_packet->index) {
            memcpy(last_packet, packet, MTU);
        }

        --count;
        if (!count) {
            break;
        }
    }

    std::cout << "All packets are received 1 " << offset << std::endl;

    while (true) {
        recvfrom(s, packet, MTU, 0, (struct sockaddr*)&csin, &csinlen);

        if (packet->index >= end) {
            std::cout << "All packets are received 2 " << packet->index << std::endl;
            break;
        }
    }
}

int Socket(int __domain, int __type, int __protocol) {
    int fd = socket(__domain, __type, __protocol);
    if (fd == -1) {
        perror("Socket creation error");
        exit(1);
    }
    return fd;
}

int Setsockopt(int __fd, int __level, int __optname, const void* __optval, socklen_t __optlen) {
    int status = setsockopt(__fd, __level, __optname, __optval, __optlen);
    if (status == -1) {
        perror("Setsockopt error");
        exit(1);
    }
    return status;
}

int Bind(int __fd, const sockaddr* __addr, socklen_t __len) {
    int status = bind(__fd, __addr, __len);
    if (status == -1) {
        perror("Binding error");
        exit(1);
    }
    return status;
}

int Connect(int __fd, const sockaddr* __addr, socklen_t __len) {
    int status = connect(__fd, __addr, __len);
    if (status == -1) {
        perror("Connecting error");
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

int Accept(int __fd, sockaddr* __restrict__ __addr, socklen_t* __restrict__ __addr_len) {
    int fd = accept(__fd, __addr, __addr_len);
    if (fd == -1) {
        perror("Accepting error");
        exit(1);
    }
    return fd;
}

ssize_t Recv(int __fd, void* __buf, size_t __n, int __flags) {
    ssize_t bytes_received = recv(__fd, __buf, __n, __flags);
    if (bytes_received == -1) {
        perror("Receiving error");
        close(__fd);
        return -1;
    }
    return bytes_received;
}

ssize_t Send(int __fd, const void* __buf, size_t __n, int __flags) {
    size_t nleft = __n;
    ssize_t nwritten;
    const char* ptr = (const char*)__buf;

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

ssize_t Sendto(int __fd, const void* __buf, size_t __n, int __flags, const struct sockaddr* __addr, socklen_t __addr_len) {
    size_t nleft = __n;
    ssize_t nwritten;
    const char* ptr = (const char*)__buf;

    while (nleft > 0) {
        if ((nwritten = sendto(__fd, ptr, nleft, __flags, __addr, __addr_len)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else {
                perror("Sendto error");
                continue;
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return __n;
}

ssize_t Recvfrom(int __fd, void* __buf, size_t __n, int __flags, struct sockaddr* __addr, socklen_t* __addr_len) {
    size_t nleft = __n;
    ssize_t nread;
    const char* ptr = (const char*)__buf;

    while (nleft > 0) {
        if ((nread = recvfrom(__fd, (void*)ptr, nleft, __flags, __addr, __addr_len)) <= 0) {
            if (nread < 0 && errno == EINTR)
                nread = 0;
            else {
                perror("Recvfrom error");
                continue;
            }
        }
        nleft -= nread;
        ptr += nread;
    }

    return __n;
}