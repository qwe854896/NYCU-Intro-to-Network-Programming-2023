#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

int main(int argc, char *argv[]) {
    /* Argument */
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <path_to_read_files> <total_number_of_files> <port> <server_ip_address>" << std::endl;
        return 1;
    }

    std::string path_to_read_files = argv[1];
    size_t total_number_of_files = std::atoi(argv[2]);
    int port = strtol(argv[3], NULL, 0);
    const char *server_ip_address = argv[4];

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Socket */
    int s;
    struct sockaddr_in sin;

    // struct timeval tv;
    // tv.tv_sec = CLT_RECV_TIMEOUT / 1000000;
    // tv.tv_usec = CLT_RECV_TIMEOUT % 1000000;

    // Create UDP socket
    s = Socket(AF_INET, SOCK_DGRAM, 0);
    // Setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Set up server address struct
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip_address, &sin.sin_addr) != 1) {
        return -fprintf(stderr, "** cannot convert IPv4 address for %s\n", argv[1]);
    }
    sin.sin_port = htons(port);

    /* FILE */

    // WE VIEW THE PROTOCOL HEADER AS A FILE, WHICH HAS INDEX 0
    // INDEX 0    -> SPECIAL_HEADER
    // INDEX 1    -> 000000
    // INDEX 2    -> 000001
    // ...
    // INDEX 1000 -> 000999

    // FILENAME
    filename[0] = SPECIAL_HEADER;
    for (size_t i = 0; i < total_number_of_files; ++i) {
        filename[i + 1] = path_to_read_files + "/" + index2string(i);
    }

    // FILE LENGTH
    std::vector<uint32_t> file_length(total_number_of_files + 1);

    file_length[0] = 4 * total_number_of_files;
    for (size_t i = 1; i <= total_number_of_files; ++i) {
        struct stat st;
        stat(filename[i].c_str(), &st);
        file_length[i] = st.st_size;
    }

    /* PACKET */

    // PACKET METADATA
    std::vector<PacketMetadata> packet_metadata;
    file_length2packet_metadata(file_length, packet_metadata);
    for (size_t i = 0; i < packet_metadata.size(); ++i) {
        packet_metadata[i].index = i;
    }

    // PACKET HEADER
    Header *file_0 = (Header *)header;
    for (size_t i = 0; i < total_number_of_files; ++i) {
        file_0->size[i] = file_length[i + 1];
    }

    // Ensure the first special_packet_size metadata are sent correctly
    size_t start = 0;
    size_t end = (file_length[0] + PKT_SIZE - 1) / PKT_SIZE;
    std::queue<size_t> queued_packets_index;

    // Ensure the remaining are sent correctly
    while (start != packet_metadata.size()) {
        for (size_t i = start; i < end; ++i) {
            queued_packets_index.push(i);
        }
        ensure_queued_packets_are_sent(s, sin, packet_metadata, file_length, queued_packets_index);
        start = end;
        end = std::min(start + ACK_BIT, packet_metadata.size());
    }
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include "common.h"

#define NIPQUAD(s) ((unsigned char *)&s)[0], \
                   ((unsigned char *)&s)[1], \
                   ((unsigned char *)&s)[2], \
                   ((unsigned char *)&s)[3]

#endif