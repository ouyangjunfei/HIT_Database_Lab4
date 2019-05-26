//
// Created by Ouyangjunfei on 2019/5/13.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include "initDataBlocks.h"

int createRelation(int block_num, char relation_name, int first_min, int first_max, int second_min, int second_max,
                   Buffer *buf) {
    for (int i = 0; i < block_num; i++) {
        unsigned int addr = relation_name * BASE_ADDR;
        addr += i;
        srand(time(0) + i);

        Block *pblock = (Block *) getNewBlockInBuffer(buf);
        memset(pblock, 0, sizeof(Block));
        int *ptr = (int *) pblock;
        for (int j = 0; j < 7; j++) {
            *ptr = rand() % first_max + first_min;
            *(ptr + 1) = rand() % second_max + second_min;
            ptr += 2;
        }
        if (i != block_num - 1) {
            *(ptr + 1) = addr + 1;
        } else {
            *(ptr + 1) = 0;
        }
        writeBlockToDisk((unsigned char *) pblock, addr, buf);
    }
    return 0;
}

void sortSingleBlk(unsigned int addr, char field, Buffer *buf) {
    unsigned char *blkPtr = readBlockFromDisk(addr, buf);
    int tuples[N] = {0};
    int offset = 0;
    int index = 0;
    if (field == 'A' || field == 'C') {
        offset = 0;
    } else if (field == 'B' || field == 'D') {
        offset = 1;
    }
    unsigned int *pInt = NULL;
    for (pInt = (unsigned int *) blkPtr;
         pInt < (unsigned int *) (blkPtr + buf->blkSize - 2 * sizeof(int32_t)); pInt += 2) {
        tuples[index++] = *pInt;
        tuples[index++] = *(pInt + 1);
    }
    for (int i = 0; i < N / 2; i++) {
        int min = i;
        int temp1, temp2;
        for (int j = i + 1; j < N / 2; j++) {
            if (tuples[2 * j + offset] < tuples[2 * min + offset]) {
                if (field == 'A' || field == 'C') {
                    temp1 = tuples[2 * j];
                    tuples[2 * j] = tuples[2 * min];
                    tuples[2 * min] = temp1;
                    temp2 = tuples[2 * j + 1];
                    tuples[2 * j + 1] = tuples[2 * min + 1];
                    tuples[2 * min + 1] = temp2;
                } else if (field == 'B' || field == 'D') {
                    temp1 = tuples[2 * j + 1];
                    tuples[2 * j + 1] = tuples[2 * min + 1];
                    tuples[2 * min + 1] = temp1;
                    temp2 = tuples[2 * j];
                    tuples[2 * j] = tuples[2 * min];
                    tuples[2 * min] = temp2;
                }
            }
        }
    }
    index = 0;
    for (pInt = (unsigned int *) blkPtr;
         pInt < (unsigned int *) (blkPtr + buf->blkSize - 2 * sizeof(int32_t)); pInt += 2) {
        *pInt = tuples[index++];
        *(pInt + 1) = tuples[index++];
    }
    writeBlockToDisk(blkPtr, addr, buf);
}

void sortRelationship(char relation_name, char field, int relation_size, Buffer *buf) {
    unsigned int addr;
    for (int i = 0; i < relation_size; i++) {
        addr = relation_name * BASE_ADDR + i;
        sortSingleBlk(addr, field, buf);
    }
}

void externalSort(char relation_name, char field, int relation_size, Buffer *buf) {
    if (relation_name == 'R') {
        unsigned int *blkPtr[7];    //存储当前缓冲区块首地址
        int sort_cursor[7] = {0};        //游标指针，用于维护某块当前指向位置
        int i;
        for (i = 0; i < 7; i++) {
            blkPtr[i] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
        }
        unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
        memset(pOut, 0, sizeof(Block));
        int out_count = 0;
        unsigned int addr = TEMP_ADDR;
        /**
         * 对关系R的属性A采取772的读块方式
         * <br>生成三条有序序列
         * <br>再将三条有序序列放入缓冲区排序
         */
        if (field == 'A') {
            /**
             * 第一个7块
             */
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 7; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }

                for (int j = 1; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }

                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;

                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            for (int k = 0; k < 7; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            for (; i < 14; i++) {
                blkPtr[i - 7] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            memset(sort_cursor, 0, sizeof(sort_cursor));
            /**
             * 第二个7块
             */
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 7; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            for (int k = 0; k < 7; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            for (; i < 16; i++) {
                blkPtr[i - 14] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            memset(sort_cursor, 0, sizeof(sort_cursor));
            /**
             * 第三个2块
             */
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 2; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 2) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 2; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 2; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            for (int k = 0; k < 2; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            /**
             * 合并操作部分
             */
            int merge_cursor[3] = {0};
            int blkStatus[3] = {0};
            unsigned int addrs[3] = {TEMP_ADDR, TEMP_ADDR + 7, TEMP_ADDR + 14};
            addr = SORTED_ADDR_R;
            blkPtr[0] = (unsigned int *) readBlockFromDisk(addrs[0]++, buf);
            blkPtr[1] = (unsigned int *) readBlockFromDisk(addrs[1]++, buf);
            blkPtr[2] = (unsigned int *) readBlockFromDisk(addrs[2]++, buf);

            while (1) {
                if (blkStatus[0] == 6 && blkStatus[1] == 6 && blkStatus[2] == 1 && merge_cursor[0] == 7 &&
                    merge_cursor[1] == 7 && merge_cursor[2] == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 3; j++) {
                    if (merge_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 3; j++) {
                    if (merge_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * merge_cursor[j]) < *(blkPtr[min] + 2 * merge_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * merge_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * merge_cursor[min] + 1);
                merge_cursor[min]++;
                out_count++;
                if (merge_cursor[min] == 7) {
                    if (min < 2 && blkStatus[min] < 6) {
                        freeBlockInBuffer((unsigned char *) blkPtr[min], buf);
                        blkPtr[min] = (unsigned int *) readBlockFromDisk(addrs[min]++, buf);
                        merge_cursor[min] = 0;
                        blkStatus[min]++;
                    }
                    if (min == 2 && blkStatus[min] < 1) {
                        freeBlockInBuffer((unsigned char *) blkPtr[min], buf);
                        blkPtr[min] = (unsigned int *) readBlockFromDisk(addrs[min]++, buf);
                        merge_cursor[min] = 0;
                        blkStatus[min]++;
                    }
                }
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            freeBlockInBuffer((unsigned char *) pOut, buf);
            for (i = 0; i < 7; i++) {
                freeBlockInBuffer((unsigned char *) (blkPtr[i]), buf);
            }
        }
        if (field == 'B') {

        }
    }
    if (relation_name == 'S') {
        unsigned int *blkPtr[7];    //存储当前缓冲区块首地址
        int sort_cursor[7] = {0};        //游标指针，用于维护某块当前指向位置
        int i;
        unsigned int *pOut = (unsigned int *) getNewBlockInBuffer(buf);
        memset(pOut, 0, sizeof(Block));
        int out_count = 0;
        unsigned int addr = 2 * TEMP_ADDR;
        /**
         * 对关系S的属性A采取77774的读块方式
         * <br>生成五条有序序列
         * <br>再将五条有序序列放入缓冲区排序
         */
        if (field == 'C') {
            /**
             * 第一个7块
             */
            for (i = 0; i < 7; i++) {
                blkPtr[i] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 7; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }

                for (int j = 1; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }

                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;

                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            for (int k = 0; k < 7; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            /**
             * 第二个7块
             */
            for (; i < 14; i++) {
                blkPtr[i - 7] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            memset(sort_cursor, 0, sizeof(sort_cursor));
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 7; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            for (int k = 0; k < 7; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            /**
             * 第三个7块
             */
            for (; i < 21; i++) {
                blkPtr[i - 14] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            memset(sort_cursor, 0, sizeof(sort_cursor));
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 7; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }

            }
            for (int k = 0; k < 7; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            /**
             * 第四个7块
             */
            for (; i < 28; i++) {
                blkPtr[i - 21] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            memset(sort_cursor, 0, sizeof(sort_cursor));
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 7; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 7; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }

            }
            for (int k = 0; k < 7; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }
            /**
             * 第五个4块
             */
            for (; i < 32; i++) {
                blkPtr[i - 28] = (unsigned int *) readBlockFromDisk(relation_name * BASE_ADDR + i, buf);
            }
            memset(sort_cursor, 0, sizeof(sort_cursor));
            while (1) {
                int finished_count = 0;
                for (int temp = 0; temp < 4; temp++) {
                    if (sort_cursor[temp] == 7) {
                        finished_count++;
                    }
                }
                if (finished_count == 4) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 4; j++) {
                    if (sort_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 4; j++) {
                    if (sort_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * sort_cursor[j]) < *(blkPtr[min] + 2 * sort_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * sort_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * sort_cursor[min] + 1);
                sort_cursor[min]++;
                out_count++;
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }

            }
            for (int k = 0; k < 4; k++) {
                freeBlockInBuffer((unsigned char *) blkPtr[k], buf);
            }

            /**
             * 合并操作部分
             */
            int merge_cursor[5] = {0};
            int blkStatus[5] = {0};
            unsigned int addrs[5] = {2 * TEMP_ADDR, 2 * TEMP_ADDR + 7, 2 * TEMP_ADDR + 14, 2 * TEMP_ADDR + 21,
                                     2 * TEMP_ADDR + 28};
            addr = SORTED_ADDR_S;
            blkPtr[0] = (unsigned int *) readBlockFromDisk(addrs[0]++, buf);
            blkPtr[1] = (unsigned int *) readBlockFromDisk(addrs[1]++, buf);
            blkPtr[2] = (unsigned int *) readBlockFromDisk(addrs[2]++, buf);
            blkPtr[3] = (unsigned int *) readBlockFromDisk(addrs[3]++, buf);
            blkPtr[4] = (unsigned int *) readBlockFromDisk(addrs[4]++, buf);

            while (1) {
                if (blkStatus[0] == 6 && blkStatus[1] == 6 && blkStatus[2] == 6 && blkStatus[3] == 6 &&
                    blkStatus[4] == 3 && merge_cursor[0] == 7 &&
                    merge_cursor[1] == 7 && merge_cursor[2] == 7 && merge_cursor[3] == 7 && merge_cursor[4] == 7) {
                    break;
                }
                int min = 0;
                for (int j = 0; j < 5; j++) {
                    if (merge_cursor[j] != 7) {
                        min = j;
                        break;
                    }
                }
                for (int j = 1; j < 5; j++) {
                    if (merge_cursor[j] != 7) {
                        if (*(blkPtr[j] + 2 * merge_cursor[j]) < *(blkPtr[min] + 2 * merge_cursor[min])) {
                            min = j;
                        }
                    }
                }
                *(pOut + 2 * out_count) = *(blkPtr[min] + 2 * merge_cursor[min]);
                *(pOut + 2 * out_count + 1) = *(blkPtr[min] + 2 * merge_cursor[min] + 1);
                merge_cursor[min]++;
                out_count++;
                if (merge_cursor[min] == 7) {
                    if (min < 4 && blkStatus[min] < 6) {
                        freeBlockInBuffer((unsigned char *) blkPtr[min], buf);
                        blkPtr[min] = (unsigned int *) readBlockFromDisk(addrs[min]++, buf);
                        merge_cursor[min] = 0;
                        blkStatus[min]++;
                    }
                    if (min == 4 && blkStatus[min] < 3) {
                        freeBlockInBuffer((unsigned char *) blkPtr[min], buf);
                        blkPtr[min] = (unsigned int *) readBlockFromDisk(addrs[min]++, buf);
                        merge_cursor[min] = 0;
                        blkStatus[min]++;
                    }
                }
                if (out_count == 7) {
                    writeBlockToDisk((unsigned char *) pOut, addr++, buf);
                    pOut = (unsigned int *) getNewBlockInBuffer(buf);
                    memset(pOut, 0, sizeof(Block));
                    out_count = 0;
                }
            }
            freeBlockInBuffer((unsigned char *) pOut, buf);
            for (i = 0; i < 7; i++) {
                freeBlockInBuffer((unsigned char *) (blkPtr[i]), buf);
            }
        }
        if (field == 'D') {

        }
    }
}

int locateFirstShown(char relation_name, int number, Buffer *buf) {
    int relation_length = (relation_name == 'R') ? R_BLOCK : S_BLOCK;
    int low = 0;
    int high = relation_length * 7 - 1;
    int mid;

    while (low <= high) {
        mid = low + (high - low) / 2;

        if (number <= getNum(relation_name, mid, buf))
            high = mid - 1;
        else
            low = mid + 1;

    }
    if (low < relation_length * 7 && getNum(relation_name, low, buf) == number)
        return low;
    else
        return -1;
}

int getNum(char relation_name, int cursor, Buffer *buf) {
    unsigned int addr = (relation_name == 'R') ? SORTED_ADDR_R : SORTED_ADDR_S;
    addr += cursor / 7;
    unsigned int *blkPtr = (unsigned int *) readBlockFromDisk(addr, buf);
    int temp = *(blkPtr + 2 * (cursor % 7));
    freeBlockInBuffer((unsigned char *) blkPtr, buf);
    return temp;
}

int hasHash(int hash_addr_prefix, int number) {
    return hash_addr_prefix + 10 * number;
}

int hashSingleBlock(char relation_name, unsigned int addr, Buffer *buf) {
    int count = 0;
    int hash_addr_prefix = (relation_name == 'R') ? HASH_ADDR_R : HASH_ADDR_S;
    unsigned int *pInt = (unsigned int *) readBlockFromDisk(addr, buf);
    int tuple_count = 0;
    while (tuple_count < 7) {
        int key = *(pInt + 2 * tuple_count);
        int hash_addr = hasHash(hash_addr_prefix, key);
        unsigned int *pOut;
        int out_count = 0;
        if ((pOut = (unsigned int *) readBlockFromDisk(hash_addr, buf)) == NULL) {
            pOut = (unsigned int *) getNewBlockInBuffer(buf);
            memset(pOut, 0, sizeof(Block));
        } else {
            while (*(pOut + out_count)) {
                out_count++;
                if (out_count == 16) {
                    hash_addr++;
                    //限制条件，目前不会达到
                    if (hash_addr == hasHash(hash_addr_prefix, key) + 10) {
                        break;
                    }
                    freeBlockInBuffer((unsigned char *) pOut, buf);
                    if ((pOut = (unsigned int *) readBlockFromDisk(hash_addr, buf)) == NULL) {
                        pOut = (unsigned int *) getNewBlockInBuffer(buf);
                        memset(pOut, 0, sizeof(Block));
                    }
                    out_count = 0;
                }
            }
        }
        *(pOut + out_count) = *(pInt + 2 * tuple_count + 1);
        count++;
        tuple_count++;
        writeBlockToDisk((unsigned char *) pOut, hash_addr, buf);
    }
    freeBlockInBuffer((unsigned char *) pInt, buf);
    return count;
}

int hashRelation(char relation_name, int relation_size, Buffer *buf) {
    unsigned int addr = relation_name * BASE_ADDR;
    int count = 0;
    for (int i = addr; i < addr + relation_size; i++) {
        count += hashSingleBlock(relation_name, i, buf);
    }
    return count;
}





