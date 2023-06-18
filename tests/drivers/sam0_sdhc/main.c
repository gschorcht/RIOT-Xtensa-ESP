/*
 * Copyright (C) 2016 Michel Rottleuthner
 *               2023 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the SAM0 SDHC driver
 *
 * @author      Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 * @author      Gunar Schorcht <gunar@schorcht.net>
 * @}
 */

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "byteorder.h"
#include "container.h"
#include "fmt.h"
#include "macros/units.h"
#include "shell.h"

#include "sdhc.h"

/* independent of what you specify in a r/w cmd this is the maximum number of blocks read at once.
   If you call read with a bigger blockcount the read is performed in chunks*/
#define MAX_BLOCKS_IN_BUFFER 4
#define BLOCK_PRINT_BYTES_PER_LINE 16
#define FIRST_PRINTABLE_ASCII_CHAR 0x20
#define ASCII_UNPRINTABLE_REPLACEMENT "."

sdhc_state_t sdhc_dev = {
    .dev = SDHC_DEV,
    .cd  = SDHC_CD,
    .wp  = SDHC_WP,
    .need_init = true,
};

sdhc_state_t *dev = &sdhc_dev;

uint8_t buffer[SD_MMC_BLOCK_SIZE * MAX_BLOCKS_IN_BUFFER];

static int _init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("Initializing SD Card/MMC\n");
    if (sdhc_init(dev)) {
        puts("[FAILED]");
        puts("enable debugging in sdhc.c for more information!");
        return -2;
    }
    printf("card found [OK]\n");

    return 0;
}

#define KILO    (1000UL)
#define MEGA    (1000000UL)
#define GIGA    (1000000000UL)

static void _print_size(uint64_t bytes)
{
    /* gib_frac = (bytes - gib_int * GiB) / MiB * KILO / KiB; */
    uint32_t gib_int = bytes / GiB(1);
    uint32_t gib_frac = (((bytes / MiB(1)) - (gib_int * KiB(1))) * KILO) / KiB(1);

    /* gb_frac = (bytes - gb_int * GIGA) / MEGA */
    uint32_t gb_int = bytes / GIGA;
    uint32_t gb_frac = (bytes / MEGA) - (gb_int * KILO);

    print_u64_dec( bytes );
    printf(" bytes (%" PRIu32 ",%03" PRIu32 " GiB | %" PRIu32 ",%03" PRIu32 " GB)\n", gib_int,
           gib_frac, gb_int, gb_frac);
}

static int _size(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    puts("\nCard size: ");
    _print_size(dev->sectors * SD_MMC_BLOCK_SIZE);

    return 0;
}

static int _read(int argc, char **argv)
{
    int blockaddr;
    int cnt;
    bool print_as_char = false;

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    if ((argc == 3) || (argc == 4)) {
        blockaddr = atoi(argv[1]);
        cnt = atoi(argv[2]);
        if (argc == 4 && (strcmp("-c", argv[3]) == 0)) {
            print_as_char = true;
        }
    }
    else {
        printf("usage: %s blockaddr cnt [-c]\n", argv[0]);
        return -1;
    }

    int total_read = 0;

    while (total_read < cnt) {
        int chunk_blocks = cnt - total_read;
        if (chunk_blocks > MAX_BLOCKS_IN_BUFFER) {
            chunk_blocks = MAX_BLOCKS_IN_BUFFER;
        }

        uint16_t chunks_read = 0;
        int res = sdhc_read_blocks(dev, blockaddr + total_read,
                                   buffer, chunk_blocks);
        if (res) {
            printf("read error %d (block %d/%d)\n",
                   res, total_read + chunks_read, cnt);
            return -1;
        }
        chunks_read += chunk_blocks;

        if (IS_USED(OUTPUT)) {
            for (int i = 0; i < chunk_blocks * SD_MMC_BLOCK_SIZE; i++) {

                if ((i % SD_MMC_BLOCK_SIZE) == 0) {
                    printf("BLOCK %d:\n",
                           blockaddr + total_read + i / SD_MMC_BLOCK_SIZE);
                }

                if (print_as_char) {
                    if (buffer[i] >= FIRST_PRINTABLE_ASCII_CHAR) {
                        printf("%c", buffer[i]);
                    }
                    else {
                        printf(ASCII_UNPRINTABLE_REPLACEMENT);
                    }
                }
                else {
                    printf("%02x ", buffer[i]);
                }

                if ((i % BLOCK_PRINT_BYTES_PER_LINE) == (BLOCK_PRINT_BYTES_PER_LINE - 1)) {
                    puts(""); /* line break after BLOCK_PRINT_BYTES_PER_LINE bytes */
                }

                if ((i % SD_MMC_BLOCK_SIZE) == (SD_MMC_BLOCK_SIZE - 1)) {
                    puts(""); /* empty line after each printed block */
                }
            }
        }
        total_read += chunks_read;
    }
    printf("read %d block(s) from %d [OK]\n", cnt, blockaddr);
    return 0;
}

static int _write(int argc, char **argv)
{
    int bladdr;
    char *data;
    int size;
    bool repeat_data = false;

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    if (argc == 3 || argc == 4) {
        bladdr = atoi(argv[1]);
        data = argv[2];
        size = strlen(argv[2]);
        printf("will write '%s' (%d chars) at start of block %d\n",
               data, size, bladdr);
        if (argc == 4 && (strcmp("-r", argv[3]) == 0)) {
            repeat_data = true;
            puts("the rest of the block will be filled with copies of that string");
        }
        else {
            puts("the rest of the block will be filled with zeros");
        }
    }
    else {
        printf("usage: %s blockaddr string [-r]\n", argv[0]);
        return -1;
    }

    if (size > SD_MMC_BLOCK_SIZE) {
        printf("maximum stringsize to write at once is %d ...aborting\n",
               SD_MMC_BLOCK_SIZE);
        return -1;
    }

    /* copy data to a full-block-sized buffer an fill remaining block space
     * according to -r param*/
    uint8_t write_buffer[SD_MMC_BLOCK_SIZE];
    for (unsigned i = 0; i < sizeof(write_buffer); i++) {
        if (repeat_data || ((int)i < size)) {
            write_buffer[i] = data[i % size];
        }
        else {
            write_buffer[i] = 0;
        }
    }

    int res = sdhc_write_blocks(dev, bladdr, write_buffer, 1);
    if (res) {
        printf("write error %d (wrote 1/1 blocks)\n", res);
        return -1;
    }

    printf("write block %d [OK]\n", bladdr);
    return 0;
}

static int _writem(int argc, char **argv)
{
    int bladdr;
    int cnt;

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    if (argc == 3) {
        bladdr = atoi(argv[1]);
        cnt = atoi(argv[2]);
    }
    else {
        printf("usage: %s blockaddr num\n", argv[0]);
        return -1;
    }

    /* writing cnt blocks with data from stack */
    int res = sdhc_write_blocks(dev, bladdr, (void *)&bladdr, cnt);
    if (res) {
        printf("write error %d (wrote %u/%d blocks)\n", res, 0, cnt);
        return -1;
    }

    printf("write %d blocks to %d [OK]\n", cnt, bladdr);
    return 0;
}

static int _erase(int argc, char **argv)
{
    int blockaddr;
    int cnt;

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    if (argc == 3) {
        blockaddr = atoi(argv[1]);
        cnt = atoi(argv[2]);
    }
    else {
        printf("usage: %s blockaddr cnt\n", argv[0]);
        return -1;
    }

    int res = sdhc_erase_blocks(dev, blockaddr, cnt);
    if (res) {
        printf("erase error %d\n", res);
        return -1;
    }

    printf("erase %d block(s) from %d [OK]\n", cnt, blockaddr);
    return 0;
}

static int _copy(int argc, char **argv)
{
    int src_block;
    int dst_block;
    int num_block = 1;
    uint8_t tmp_copy[SD_MMC_BLOCK_SIZE];

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    if (argc < 3) {
        printf("usage: %s src dst [num]\n", argv[0]);
        return -1;
    }

    src_block = atoi(argv[1]);
    dst_block = atoi(argv[2]);

    if (argc == 4) {
        num_block = atoi(argv[3]);
    }

    for (int i = 0; i < num_block; i++) {
        int res = sdhc_read_blocks(dev, src_block + i, tmp_copy, 1);

        if (res) {
            printf("read error %d (block %d)\n", res, src_block + i);
            return -1;
        }

        res = sdhc_write_blocks(dev, dst_block + i, tmp_copy, 1);

        if (res) {
            printf("write error %d (block %d)\n", res, dst_block + i);
            return -2;
        }

        if (IS_USED(OUTPUT) && (num_block > 1)) {
            extern ssize_t stdio_write(const void *buffer, size_t len);
            stdio_write(".", 1);
            if ((num_block % 79) == 79) {
                printf("\n");
            }
        }
    }
    if (IS_USED(OUTPUT)) {
        printf("\n");
    }

    printf("copy %d block(s) from %d to %d [OK]\n",
           num_block, src_block, dst_block);
    return 0;
}

static int _sector_count(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (dev->need_init) {
        printf("[Error] Card not initialized or not present, use init command\n");
        return -1;
    }

    printf("available sectors on card: %" PRIu32 "\n", dev->sectors);
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "init", "initializes default card", _init },
    { "size", "print card size", _size },
    { "sectors", "print sector count of card", _sector_count },
    { "read", "'read n m' reads m blocks beginning at block address n and prints the result. "
              "Append -c option to print data readable chars", _read },
    { "write", "'write n data' writes data to block n. Append -r option to "
               "repeatedly write data to complete block", _write },
    { "copy", "'copy src dst' copies block src to block dst", _copy },
    { "erase", "'erase n m' erases m blocks beginning at block address n", _erase },
    { "writem", "'write n m' writes m data blocks beginning at block address n.",
                _writem },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("SAM0 SDHC driver test application");

    puts("insert a SD Card/MMC and use 'init' command to initialize the card");
    puts("WARNING: using 'write' or 'copy' commands WILL overwrite data on your card and");
    puts("almost for sure corrupt existing filesystems, partitions and contained data!");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
