//
// Created by Ouyangjunfei on 2019/5/14.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "select.h"

void LinearSearch(char relation_name, char field_name, int number, Buffer *buf) {
    unsigned int addr;

    unsigned char *blkPtr = NULL;
    unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
    memset(pOut, 0, sizeof(Block));
    int block_count = 0;
    int tuple_count = 0;

    int offset = 0;
    if (field_name == 'A' || field_name == 'C') {
        offset = 0;
    } else if (field_name == 'B' || field_name == 'D') {
        offset = 1;
    }

    int BLOCK_SIZE = (relation_name == 'R') ? R_BLOCK : S_BLOCK;
    int PREFIX = (relation_name == 'R') ? LINEAR_ADDR_R : LINEAR_ADDR_S;

    for (int i = 0; i < BLOCK_SIZE; i++) {
        addr = relation_name * BASE_ADDR + i;
        blkPtr = readBlockFromDisk(addr, buf);

        unsigned int *pInt = NULL;
        for (pInt = (unsigned int *) blkPtr;
             pInt < (unsigned int *) (blkPtr + buf->blkSize - 2 * sizeof(int32_t)); pInt += 2) {
            if (*(pInt + offset) == number) {
                if (tuple_count == N) {
                    *(pOut + N+1) = PREFIX + block_count + 1;
                    writeBlockToDisk((unsigned char *) pOut, PREFIX + block_count, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    tuple_count = 0;
                    block_count++;
                }
                if (offset) {
                    *(pOut + tuple_count) = *pInt;
                    *(pOut + tuple_count + 1) = *(pInt + offset);
                } else {
                    *(pOut + tuple_count) = *(pInt + offset);
                    *(pOut + tuple_count + 1) = *(pInt + 1);
                }
                tuple_count += 2;
            }
        }
        freeBlockInBuffer(blkPtr, buf);
    }
    *(pOut + N+1) = 0;
    writeBlockToDisk((unsigned char *) pOut, PREFIX + block_count, buf);
}

void BinarySearch(char relation_name, char field_name, int number, Buffer *buf) {
    if (relation_name == 'R') {
        if (field_name == 'A') {
            int firstShownLocation = locateFirstShown(relation_name, number, buf);
            int block_num = firstShownLocation / 7;
            int tuple_count = firstShownLocation % 7;
            unsigned int addr = SORTED_ADDR_R;
            unsigned int out_addr = BINARY_ADDR_R;
            addr += block_num;
            unsigned char *blkPtr = readBlockFromDisk(addr++, buf);
            unsigned int *pInt = (unsigned int *) blkPtr;
            unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
            memset(pOut, 0, sizeof(Block));
            int out_count = 0;
            while (*(pInt + 2 * tuple_count) == number) {
                *(pOut + 2 * out_count) = *(pInt + 2 * tuple_count);
                *(pOut + 2 * out_count + 1) = *(pInt + 2 * tuple_count + 1);
                tuple_count++;
                out_count++;
                if (out_count == N/2) {
                    *(pOut + N+1) = out_addr + 1;
                    writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
                if (tuple_count == 7) {
                    freeBlockInBuffer((unsigned char *) pInt, buf);
                    if (addr < SORTED_ADDR_R + R_BLOCK) {
                        pInt = (unsigned int *) readBlockFromDisk(addr++, buf);
                        tuple_count = 0;
                    } else {
                        break;
                    }
                }
            }
            writeBlockToDisk((unsigned char *) pOut, out_addr, buf);
        }
    }
    if (relation_name == 'S') {
        if (field_name == 'C') {
            int firstShownLocation = locateFirstShown(relation_name, number, buf);
            int block_num = firstShownLocation / 7;
            int tuple_count = firstShownLocation % 7;
            unsigned int addr = SORTED_ADDR_S;
            unsigned int out_addr = BINARY_ADDR_S;
            addr += block_num;
            unsigned char *blkPtr = readBlockFromDisk(addr++, buf);
            unsigned int *pInt = (unsigned int *) blkPtr;
            unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
            memset(pOut, 0, sizeof(Block));
            int out_count = 0;
            while (*(pInt + 2 * tuple_count) == number) {
                *(pOut + 2 * out_count) = *(pInt + 2 * tuple_count);
                *(pOut + 2 * out_count + 1) = *(pInt + 2 * tuple_count + 1);
                tuple_count++;
                out_count++;
                if (out_count == N/2) {
                    *(pOut + N+1) = out_addr + 1;
                    writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
                if (tuple_count == 7) {
                    freeBlockInBuffer((unsigned char *) pInt, buf);
                    if (addr < SORTED_ADDR_S + S_BLOCK) {
                        pInt = (unsigned int *) readBlockFromDisk(addr++, buf);
                        tuple_count = 0;
                    } else {
                        break;
                    }
                }
            }
            writeBlockToDisk((unsigned char *) pOut, out_addr, buf);
        }
    }
}

void HashSearch(char relation_name, char field_name, int number, Buffer *buf) {
    int hash_addr_prefix = (relation_name == 'R') ? HASH_ADDR_R : HASH_ADDR_S;
    unsigned int out_addr = (relation_name == 'R') ? INDEX_ADDR_R : INDEX_ADDR_S;
    if ((relation_name == 'R' && field_name == 'A') || (relation_name == 'S' && field_name == 'C')) {
        int hash_addr = hasHash(hash_addr_prefix, number);
        unsigned int *pInt = NULL;
        if ((pInt = (unsigned int *) readBlockFromDisk(hash_addr, buf)) != NULL) {
            int tuple_count = 0;
            int out_count = 0;
            unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
            memset(pOut, 0, sizeof(Block));
            while (*(pInt + tuple_count)) {
                *(pOut + 2 * out_count) = number;
                *(pOut + 2 * out_count + 1) = *(pInt + tuple_count);
                out_count++;
                tuple_count++;
                if (out_count == N/2) {
                    *(pOut + N+1) = out_addr + 1;
                    writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
                if (tuple_count == 16) {
                    freeBlockInBuffer((unsigned char *) pInt, buf);
                    hash_addr++;
                    pInt = (unsigned int *) readBlockFromDisk(hash_addr, buf);
                    if (pInt == NULL) {
                        return;
                    }
                    tuple_count = 0;
                }
            }
            writeBlockToDisk((unsigned char *) pOut, out_addr, buf);
        } else {
            perror("Number doesn't exist!\n");
            return;
        }
    }
}