//
// Created by Ouyangjunfei on 2019/5/15.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "projection.h"

void Projection(char relation_name, char field_name, Buffer *buf) {
    unsigned int addr;

    unsigned char *blkPtr;
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
    int PREFIX = (relation_name == 'R') ? PROJECTION_ADDR_R : PROJECTION_ADDR_S;
    int BLOCK_ADDR = (relation_name == 'R') ? SORTED_ADDR_R : SORTED_ADDR_S;

    for (int i = 0; i < BLOCK_SIZE; i++) {
        addr = BLOCK_ADDR + i;
        blkPtr = readBlockFromDisk(addr, buf);
        //Block *blockin = (Block *) blkPtr;
        //Block *blockout = (Block *) pOut;

        unsigned int *pInt;
        for (pInt = (unsigned int *) blkPtr;
             pInt < (unsigned int *) (blkPtr + buf->blkSize - 2 * sizeof(int32_t)); pInt += 2) {
            if (tuple_count == N) {
                *(pOut + N+1) = PREFIX + block_count + 1;
                writeBlockToDisk((unsigned char *) pOut, PREFIX + block_count, buf);
                pOut = (unsigned int *) getNewBlockInBuffer(buf);
                memset(pOut, 0, sizeof(Block));
                tuple_count = 0;
                block_count++;
            }
            if (offset) {
                *(pOut + tuple_count) = *(pInt + offset);
            } else {
                *(pOut + tuple_count) = *pInt;
            }
            tuple_count++;
        }
        freeBlockInBuffer(blkPtr, buf);
    }
    *(pOut + N+1) = 0;
    writeBlockToDisk((unsigned char *) pOut, PREFIX + block_count, buf);
    freeBlockInBuffer((unsigned char *) pOut, buf);
}