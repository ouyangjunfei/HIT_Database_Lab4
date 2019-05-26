//
// Created by Ouyangjunfei on 2019/5/15.
//

#ifndef JOIN_H
#define JOIN_H

#include "initDataBlocks.h"

/**
 * 嵌套式连接
 * <br>目前仅对R.A和S.C实现
 * <br>一次读取4个R数据块优化算法
 *
 * @param relation_1 关系名1
 * @param relation_2 关系名2
 * @param buf 缓冲区指针
 */
void NestLoopJoin(char relation_1, char relation_2, Buffer *buf);

/**
 * 有序序列连接,包含了折半查找
 *
 * @param relation_1 关系名1
 * @param relation_2 关系名2
 * @param buf 缓冲区指针
 */
void SortMergeJoin(char relation_1, char relation_2, Buffer *buf);

/**
 * 根据哈希桶进行连接
 *
 * @param relation_1 关系名1
 * @param relation_2 关系名2
 * @param buf 缓冲区指针
 */
void HashJoin(char relation_1, char relation_2, Buffer *buf);

/**
 * 试图用双游标遍历两个关系块,但在值相等时遇到情况较为复杂，放弃实现
 *
 * @param relation_1 关系名1
 * @param relation_2 关系名2
 * @param buf 缓冲区指针
 */
void Fixed_SortMergeJoin(char relation_1, char relation_2, Buffer *buf);

#endif //JOIN_H
