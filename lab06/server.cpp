#ifndef JHC
#define JHC
#include JHC __FILE__ JHC

int main(int argc, char *argv[]) {
    /* Argument */

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <path-to-store-files> <total-number-of-files> <port>" << std::endl;
        return 1;
    }

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    const std::string path_to_store_files = argv[1];
    size_t total_number_of_files = std::atoi(argv[2]);
    int port = strtol(argv[3], NULL, 0);

    /* Socket */
    int s;
    struct sockaddr_in sin;

    // struct timeval tv;
    // tv.tv_sec = SRV_RECV_TIMEOUT / 1000000;
    // tv.tv_usec = SRV_RECV_TIMEOUT % 1000000;

    // Create UDP socket
    s = Socket(AF_INET, SOCK_DGRAM, 0);
    // Setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Set up server address struct
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    // sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin.sin_port = htons(port);

    // Bind the socket
    Bind(s, (struct sockaddr *)&sin, sizeof(sin));

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
        filename[i + 1] = path_to_store_files + "/" + index2string(i);
    }

    // FILE LENGTH (FAKE)
    std::vector<uint32_t> file_length(total_number_of_files + 1, 0);
    file_length[0] = 4 * total_number_of_files;
    file_length[1] = MTU;

    /* PACKET */
    // PACKET METADATA (HEADER)
    std::vector<PacketMetadata> packet_metadata;
    file_length2packet_metadata(file_length, packet_metadata);

    char buf[MTU];
    Packet *last_packet = (Packet *)buf;

    size_t start = 0;
    size_t end = (file_length[0] + PKT_SIZE - 1) / PKT_SIZE;
    last_packet->index = end - 1;
    send_ack_for_queued_packets(s, sin, packet_metadata, file_length, start, end - start, last_packet);
    
    // PACKET HEADER
    Header *file_0 = (Header *)header;
    uint8_t *size = (uint8_t *) file_0->size;

    // PACKET METADATA
    for (size_t i = 0; i < total_number_of_files; ++i) {
        file_length[i + 1] = *(uint32_t *)(size + 4 * i);

        // Create file of size file_length[i + 1]
        std::ofstream file(filename[i + 1], std::ios::binary);
        file.seekp(file_length[i + 1] - 1);
        file.write("", 1);
        file.close();
    }

    file_length2packet_metadata(file_length, packet_metadata);

    // Re-apply the last packet metadata
    write_buffer2files_by_metadata(file_length, packet_metadata[last_packet->index], last_packet);

    start = end;
    end = std::min(start + ACK_BIT, packet_metadata.size());
    // If all packets are received, then we can stop the server
    while (start != packet_metadata.size()) {
        send_ack_for_queued_packets(s, sin, packet_metadata, file_length, start, end - start);
        start = end;
        end = std::min(start + ACK_BIT, packet_metadata.size());
    }

    close(s);
}

#else

#pragma GCC optimize("Ofast", "unroll-loops")

#include "common.h"

#endif