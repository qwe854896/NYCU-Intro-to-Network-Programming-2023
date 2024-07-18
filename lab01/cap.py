#!/usr/bin/env python3
import socket
import struct
import sys
import time

# UDP_IP = "inp.zoolab.org"
UDP_IP = "127.0.0.1"
UDP_PORT = 10495


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <user id>")
        exit()

    id = sys.argv[1]

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)

    s.sendto(f"hello {id}".encode("utf-8"), (UDP_IP, UDP_PORT))
    data, _ = s.recvfrom(4096)

    print(data)

    chal_id = data.decode("utf-8").split()[1]
    print(chal_id)

    s.sendto(f"chals {chal_id}".encode("utf-8"), (UDP_IP, UDP_PORT))

    s.close()


if __name__ == "__main__":
    main()
