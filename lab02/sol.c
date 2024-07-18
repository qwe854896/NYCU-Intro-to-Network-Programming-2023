#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

typedef struct
{
    uint64_t magic;    /* 'BINFLAG\x00' */
    uint32_t datasize; /* in big-endian */
    uint16_t n_blocks; /* in big-endian */
    uint16_t zeros;
} __attribute((packed)) binflag_header_t;

typedef struct
{
    uint32_t offset; /* in big-endian */
    uint16_t cksum;  /* XOR'ed results of each 2-byte unit in payload */
    uint16_t length; /* ranges from 1KB - 3KB, in big-endian */
    uint8_t payload[];
} __attribute((packed)) block_t;

typedef struct
{
    uint16_t length;   /* length of the offset array, in big-endian */
    uint32_t offset[]; /* offset of the flags, in big-endian */
} __attribute((packed)) flag_t;

int read_header(FILE *file, uint32_t *datasize, uint16_t *n_blocks)
{
    fprintf(stderr, "Reading header...\n");

    binflag_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        return -1;
    }

    // Fix endianness
    *datasize = be32toh(header.datasize);
    *n_blocks = be16toh(header.n_blocks);

    fprintf(stderr, "datasize: %u\n", *datasize);
    fprintf(stderr, "n_blocks: %hu\n", *n_blocks);
    fprintf(stderr, "\n");

    return 0;
}

int read_block(FILE *file, uint8_t *D, int D_size)
{
    fprintf(stderr, "Reading block...\n");

    uint32_t offset;
    uint16_t cksum;
    uint16_t length;
    fread(&offset, sizeof(offset), 1, file);
    fread(&cksum, sizeof(cksum), 1, file);
    fread(&length, sizeof(length), 1, file);

    // Fix endianness
    offset = be32toh(offset);
    length = be16toh(length);

    block_t *block = malloc(sizeof(block_t) + length * sizeof(uint8_t));
    block->offset = offset;
    block->cksum = cksum;
    block->length = length;

    fprintf(stderr, "offset: %u\n", block->offset);
    fprintf(stderr, "cksum: %hu\n", block->cksum);
    fprintf(stderr, "length: %hu\n", block->length);
    for (int i = 0; i < block->length; i++)
    {
        fread(&block->payload[i], sizeof(uint8_t), 1, file);
    }

    cksum = 0;
    for (int i = 0; i < block->length; i += 2)
    {
        uint16_t *word = (uint16_t *)(block->payload + i);
        cksum ^= *word;
    }
    fprintf(stderr, "block checksum: %hu\n", cksum);

    if (cksum != block->cksum)
    {
        return -1;
    }

    if (block->offset + block->length > D_size)
    {
        return -1;
    }

    memcpy(D + block->offset, block->payload, block->length);

    fprintf(stderr, "\n");
    return 0;
}

int read_flags(FILE *file, uint8_t *D, int D_size)
{
    uint16_t length;
    fread(&length, sizeof(length), 1, file);

    // Fix endianness
    length = be16toh(length);

    flag_t *flags = malloc(sizeof(flag_t) + length * sizeof(uint32_t));
    flags->length = length;

    for (int i = 0; i < flags->length; i++)
    {
        uint32_t offset;
        fread(&offset, sizeof(offset), 1, file);
        flags->offset[i] = be32toh(offset);

        printf("%02x%02x", *(D + flags->offset[i]), *(D + flags->offset[i] + 1));
    }

    fprintf(stderr, "\n");
    return 0;
}

int read_bin(char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        return -1;
    }

    uint32_t datasize;
    uint16_t n_blocks;
    read_header(file, &datasize, &n_blocks);

    uint8_t *D = calloc(datasize, 1);

    for (int i = 0; i < n_blocks; i++)
    {
        if (read_block(file, D, datasize) < 0)
        {
            fprintf(stderr, "Error reading block %d\n", i);
            continue;
        }
    }

    read_flags(file, D, datasize);

    free(D);
    fclose(file);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    read_bin(argv[1]);
    return 0;
}