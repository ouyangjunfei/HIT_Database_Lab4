//
// Created by Ouyangjunfei on 2019/5/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "join.h"

void NestLoopJoin(char relation_1, char relation_2, Buffer *buf) {
    unsigned int *r1Ptr[4];   //关系1利用的缓冲区块
    unsigned int *blkPtr;
    unsigned int addr1 = relation_1 * BASE_ADDR;
    unsigned int addr2 = relation_2 * BASE_ADDR;
    unsigned int out_addr = NLJ_ADDR;
    unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
    memset(pOut, 0, sizeof(Block));
    int out_count = 0;
    for (int i = 0; i < R_BLOCK / 4; i++) {
        for (int j = 0; j < 4; j++) {
            r1Ptr[j] = (unsigned int *) readBlockFromDisk(addr1++, buf);
        }
        while (addr2 < relation_2 * BASE_ADDR + S_BLOCK) {
            blkPtr = (unsigned int *) readBlockFromDisk(addr2++, buf);
            unsigned int *pInt1;
            unsigned int *pInt2;
            for (pInt2 = blkPtr; pInt2 < blkPtr + N; pInt2 += 2) {
                for (int k = 0; k < 4; k++) {
                    for (pInt1 = r1Ptr[k]; pInt1 < r1Ptr[k] + N; pInt1 += 2) {
                        if (*pInt1 == *pInt2) {
                            *(pOut + 4 * out_count) = *pInt1;
                            *(pOut + 4 * out_count + 1) = *(pInt1 + 1);
                            *(pOut + 4 * out_count + 2) = *pInt2;
                            *(pOut + 4 * out_count + 3) = *(pInt2 + 1);
                            out_count++;
                        }
                        if (out_count == 4) {
                            writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                            out_count = 0;
                            pOut = (unsigned int *) getNewBlockInBuffer(buf);
                            memset(pOut, 0, sizeof(Block));
                        }
                    }
                }
            }
            freeBlockInBuffer((unsigned char *) blkPtr, buf);
        }
        addr2 = relation_2 * BASE_ADDR;
        for (int j = 0; j < 4; j++) {
            freeBlockInBuffer((unsigned char *) r1Ptr[j], buf);
        }
    }
}

void SortMergeJoin(char relation_1, char relation_2, Buffer *buf) {
    if (relation_1 == 'R' && relation_2 == 'S') {
        unsigned int *blkPtr_1;
        unsigned int *blkPtr_2;
        unsigned int addr1 = SORTED_ADDR_R;
        unsigned int addr2 = SORTED_ADDR_S;
        unsigned int out_addr = SMJ_ADDR;
        unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
        memset(pOut, 0, sizeof(Block));
        int out_count = 0;
        int r1_count = 0;
        int r2_count = 0;
        while (addr1 < SORTED_ADDR_R + R_BLOCK) {
            blkPtr_1 = (unsigned int *) readBlockFromDisk(addr1++, buf);
            while (r1_count < 7) {
                int key = *(blkPtr_1 + 2 * r1_count);
                int firstShownLocation;
                if (key >= 20) {
                    /**
                     * 此步骤消耗大量IO,考虑原因可能是原始数据量较小
                     * <br>折半查找的IO占比太大
                     */
                    if ((firstShownLocation = locateFirstShown('S', key, buf)) != -1) {
                        int block_num = firstShownLocation / 7;
                        r2_count = firstShownLocation % 7;
                        addr2 += block_num;
                        blkPtr_2 = (unsigned int *) readBlockFromDisk(addr2++, buf);
                        while (key == *(blkPtr_2 + 2 * r2_count)) {
                            *(pOut + 4 * out_count) = *(blkPtr_1 + 2 * r1_count);
                            *(pOut + 4 * out_count + 1) = *(blkPtr_1 + 2 * r1_count + 1);
                            *(pOut + 4 * out_count + 2) = *(blkPtr_2 + 2 * r2_count);
                            *(pOut + 4 * out_count + 3) = *(blkPtr_2 + 2 * r2_count + 1);
                            out_count++;
                            r2_count++;
                            if (r2_count == 7) {
                                freeBlockInBuffer((unsigned char *) blkPtr_2, buf);
                                blkPtr_2 = (unsigned int *) readBlockFromDisk(addr2++, buf);
                                r2_count = 0;
                            }
                            if (out_count == 4) {
                                writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                                out_count = 0;
                                pOut = (unsigned int *) getNewBlockInBuffer(buf);
                                memset(pOut, 0, sizeof(Block));
                            }
                        }
                        freeBlockInBuffer((unsigned char *) blkPtr_2, buf);
                        addr2 = SORTED_ADDR_S;
                    }
                }
                r1_count++;
            }
            freeBlockInBuffer((unsigned char *) blkPtr_1, buf);
            r1_count = 0;
        }
    }
}

void HashJoin(char relation_1, char relation_2, Buffer *buf) {
    unsigned int addr_1 = (relation_1 == 'R') ? HASH_ADDR_R : HASH_ADDR_S;
    unsigned int addr_2 = (relation_2 == 'S') ? HASH_ADDR_S : HASH_ADDR_R;
    int out_addr = HJ_ADDR;
    int out_count = 0;
    unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
    memset(pOut, 0, sizeof(Block));
    for (int i = R_A_MIN; i <= R_A_MAX; i++) {
        unsigned int r_addr = hasHash(addr_1, i);
        unsigned int *pR = NULL;
        if ((pR = (unsigned int *) readBlockFromDisk(r_addr, buf)) != NULL) {
            unsigned int *pS = NULL;
            unsigned int s_addr = hasHash(addr_2, i);
            if ((pS = (unsigned int *) readBlockFromDisk(s_addr, buf)) != NULL) {
                int r_count = 0;
                int s_count = 0;
                while (*(pR + r_count)) {
                    while (*(pS + s_count)) {
                        *(pOut + 4 * out_count) = i;
                        *(pOut + 4 * out_count + 1) = *(pR + r_count);
                        *(pOut + 4 * out_count + 2) = i;
                        *(pOut + 4 * out_count + 3) = *(pS + s_count);
                        s_count++;
                        out_count++;
                        if (out_count == 4) {
                            writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                            out_count = 0;
                            pOut = (unsigned int *) getNewBlockInBuffer(buf);
                            memset(pOut, 0, sizeof(Block));
                        }
                    }
                    s_count = 0;
                    r_count++;
                }
                freeBlockInBuffer((unsigned char *) pS, buf);
            }
            freeBlockInBuffer((unsigned char *) pR, buf);
        }
    }
}

void Fixed_SortMergeJoin(char relation_1, char relation_2, Buffer *buf) {
    if (relation_1 == 'R' && relation_2 == 'S') {
        unsigned int *blkPtr_1;
        unsigned int *blkPtr_2;
        unsigned int addr1 = SORTED_ADDR_R;
        unsigned int addr2 = SORTED_ADDR_S;
        unsigned int out_addr = FSMJ_ADDR;
        unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
        memset(pOut, 0, sizeof(Block));
        int out_count = 0;
        int r1_count = 0;
        int r2_count = 0;
        blkPtr_1 = (unsigned int *) readBlockFromDisk(addr1++, buf);
        blkPtr_2 = (unsigned int *) readBlockFromDisk(addr2++, buf);
        while (addr1 < SORTED_ADDR_R + R_BLOCK && addr2 < SORTED_ADDR_S + S_BLOCK) {
            if (*(blkPtr_1 + 2 * r1_count) < *(blkPtr_2 + 2 * r2_count)) {
                r1_count++;
                if (r1_count == 7) {
                    freeBlockInBuffer((unsigned char *) blkPtr_1, buf);
                    blkPtr_1 = (unsigned int *) readBlockFromDisk(addr1++, buf);
                    r1_count = 0;
                    if (blkPtr_1 == NULL) {
                        break;
                    }
                }
            } else if (*(blkPtr_1 + 2 * r1_count) > *(blkPtr_2 + 2 * r2_count)) {
                r2_count++;
                if (r2_count == 7) {
                    freeBlockInBuffer((unsigned char *) blkPtr_2, buf);
                    blkPtr_2 = (unsigned int *) readBlockFromDisk(addr2++, buf);
                    r2_count = 0;
                    if (blkPtr_2 == NULL) {
                        break;
                    }
                }
            } else {
                //TODO:双游标的搜索,当有多组元组值相等时，涉及块的读取，没写出来
                int r1_cursor = r1_count;
                int r2_cursor = r2_count;
                int target_value = *(blkPtr_1 + 2 * r1_count);
                while (*(blkPtr_1 + 2 * r1_cursor) == target_value) {
                    int addr2_header = addr2 - 1;
                    while (*(blkPtr_2 + 2 * r2_cursor) == target_value) {
                        *(pOut + 4 * out_count) = target_value;
                        *(pOut + 4 * out_count + 1) = *(blkPtr_1 + 2 * r1_cursor + 1);
                        *(pOut + 4 * out_count + 2) = target_value;
                        *(pOut + 4 * out_count + 3) = *(blkPtr_2 + 2 * r2_cursor + 1);
                        r2_cursor++;
                        out_count++;
                        if (out_count == 4) {
                            writeBlockToDisk((unsigned char *) pOut, out_addr++, buf);
                            out_count = 0;
                            pOut = (unsigned int *) getNewBlockInBuffer(buf);
                            memset(pOut, 0, sizeof(Block));
                        }
                        if (r2_cursor == 7) {
                            freeBlockInBuffer((unsigned char *) blkPtr_2, buf);
                            blkPtr_2 = (unsigned int *) readBlockFromDisk(addr2++, buf);
                            r2_cursor = 0;
                            if (blkPtr_2 == NULL) {
                                break;
                            }
                        }
                    }
                    r1_cursor++;
                    if (r1_cursor == 7) {
                        freeBlockInBuffer((unsigned char *) blkPtr_1, buf);
                        blkPtr_1 = (unsigned int *) readBlockFromDisk(addr1++, buf);
                        r1_cursor = 0;
                        if (blkPtr_1 == NULL) {
                            break;
                        }
                    }
                    if (*(blkPtr_1 + 2 * r1_cursor) == target_value) {
                        if (addr2 > addr2_header + 1) {
                            addr2 = addr2_header;
                            freeBlockInBuffer((unsigned char *) blkPtr_2, buf);
                            blkPtr_2 = (unsigned int *) readBlockFromDisk(addr2++, buf);
                        }
                        r2_cursor = r2_count;
                    } else {
                        r2_count = r2_cursor;
                    }
                }
                r1_count = r1_cursor;
            }
        }
    }
}