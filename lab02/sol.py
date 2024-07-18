#!/usr/bin/env python3

import sys
import struct


class binflag_header_t:
    def __init__(self, magic, datasize, n_blocks, zeros):
        self.magic = magic
        self.datasize = datasize
        self.n_blocks = n_blocks
        self.zeros = zeros


class block_t:
    def __init__(self, offset, cksum, length, payload):
        self.offset = offset
        self.cksum = cksum
        self.length = length
        self.payload = payload


class flag_t:
    def __init__(self, length, offset):
        self.length = length
        self.offset = offset


def read_header(file):
    header = binflag_header_t(*struct.unpack(">8sIHH", file.read(16)))
    return header


def read_block(file, D):
    offset, cksum, length = struct.unpack(">LHH", file.read(8))
    payload = file.read(length)
    block = block_t(offset, cksum, length, payload)

    cksum = 0
    for i in range(0, block.length, 2):
        (word,) = struct.unpack(">H", block.payload[i : i + 2])
        cksum ^= word

    if cksum != block.cksum:
        return -1

    if block.offset + block.length > len(D):
        return -1

    D[block.offset : block.offset + block.length] = block.payload
    return 0


def read_flags(file, D):
    (length,) = struct.unpack(">H", file.read(2))
    flags = flag_t(length, struct.unpack(f">{length}L", file.read(length * 4)))

    for offset in flags.offset:
        print(f"{D[offset]:02x}{D[offset+1]:02x}", end="")

    return 0


def read_bin(filename):
    with open(filename, "rb") as file:
        header = read_header(file)
        D = bytearray(header.datasize)

        for i in range(header.n_blocks):
            if read_block(file, D) < 0:
                print(f"Error reading block {i}", file=sys.stderr)
                continue

        read_flags(file, D)


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <filename>", file=sys.stderr)
        return -1

    read_bin(sys.argv[1])
    return 0


if __name__ == "__main__":
    main()
