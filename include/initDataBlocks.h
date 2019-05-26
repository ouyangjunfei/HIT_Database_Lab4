//
// Created by Ouyangjunfei on 2019/5/13.
//

#ifndef INITDATABLOCKS_H
#define INITDATABLOCKS_H

#include <stdint.h>
#include "extmem.h"

#define N 14

#define R_A_MIN 1
#define R_A_MAX 40

#define BASE_ADDR 1000
#define LINEAR_ADDR_R 1000
#define LINEAR_ADDR_S 2000
#define BINARY_ADDR_R 3000
#define BINARY_ADDR_S 4000
#define INDEX_ADDR_R 5000
#define INDEX_ADDR_S 6000
#define SORTED_ADDR_R 7000
#define SORTED_ADDR_S 8000
#define PROJECTION_ADDR_R 10000
#define PROJECTION_ADDR_S 20000
#define NLJ_ADDR 100000
#define SMJ_ADDR 200000
#define HJ_ADDR 300000
#define FSMJ_ADDR 400000
#define TEMP_ADDR 1000000
#define HASH_ADDR_R 10000000
#define HASH_ADDR_S 20000000

static int R_BLOCK = 16;
static int S_BLOCK = 32;


typedef struct tagTuple {
    int32_t field1;     //元组的第一个属性，4字节
    int32_t field2;     //元组的第二个属性，4字节
} Tuple;

typedef struct tagBlock {
    Tuple t1;       //第一个元组
    Tuple t2;       //...
    Tuple t3;
    Tuple t4;
    Tuple t5;
    Tuple t6;
    Tuple t7;       //第七个元组
    int32_t nop;
    int32_t pNextBlock;     //指向下一个磁盘块的地址
} Block;


/**
 * 创建关系数据块
 *
 * @param block_num 关系的块数
 * @param relation_name 关系名
 * @param first_min 第一个属性的下界
 * @param first_max 第一个属性的上界
 * @param second_min 第二个属性的下届
 * @param second_max 第二个属性的上界
 * @param buf 缓冲区指针
 * @return 成功返回0
 */
int createRelation(int block_num, char relation_name, int first_min, int first_max, int second_min, int second_max,
                   Buffer *buf);

/**
 * 对已经创建完成的单个数据块内元组进行排序
 *
 * @param addr 块地址
 * @param field 属性
 * @param buf 缓冲区指针
 */
void sortSingleBlk(unsigned int addr, char field, Buffer *buf);

/**
 * 按照关系的属性对该关系所有单个磁盘块进行块内排序
 *
 * @param relation_name 关系名
 * @param field 属性名
 * @param relation_size 关系大小
 * @param buf 缓冲区指针
 */
void sortRelationship(char relation_name, char field, int relation_size, Buffer *buf);

/**
 * 此部分是按照关系的属性从小到大排序，目前仅实现对R的A属性和S的C属性排列
 * <br>其中1000000开头表示R合并过程的缓存磁盘块，2000000表示S合并过程的缓存磁盘块
 * <br>3000开头为R合并最终有序结果，4000开头为S合并最终有序结果
 *
 * @param relation_name 待排序的关系名
 * @param field 按照哪个属性进行排序
 * @param relation_size 关系的块数
 * @param buf 缓冲区指针
 */
void externalSort(char relation_name, char field, int relation_size, Buffer *buf);

/**
 * 按照二分查找的思想,在有序序列中得到大小为number的元组首次出现的位置
 * <br>此函数本身不包含IO,调用getNum()函数包含IO
 * <br>现仅对元组第一个属性有效
 *
 * @param relation_name 关系名
 * @param number 被定位的数值
 * @param buf 缓冲区指针
 * @return 首次出现的位置
 */
int locateFirstShown(char relation_name, int number, Buffer *buf);

/**
 * 根据关系名和元组位置返回cursor位置的元组第一个属性的值
 *
 * @param relation_name 关系名
 * @param cursor 元组在序列中的位置
 * @param buf 缓冲区指针
 * @return 第一个属性的值
 */
int getNum(char relation_name, int cursor, Buffer *buf);

/**
 * 自定义的哈希函数,规则为哈希前缀+10*被哈希数值
 * <br>其中系数10是为了给相同的数值留存哈希空间,避免哈希碰撞
 *
 * @param hash_addr_prefix 哈希前缀
 * @param number 被哈希数值
 * @return 哈希值
 */
int hasHash(int hash_addr_prefix, int number);

/**
 * 对关系的单个块元组第一项属性进行哈希构造,构造结果为若干哈希地址的数据块
 *
 * @param relation_name 关系名
 * @param addr 原关系的数据块地址
 * @param buf 缓冲区指针
 * @return 该块被哈希的元组数
 */
int hashSingleBlock(char relation_name, unsigned int addr, Buffer *buf);

/**
 * 对关系的所有块进行哈希构造
 *
 * @param relation_name 关系名
 * @param relation_size 关系大小
 * @param buf 缓冲区指针
 * @return 被哈希的总元组数量
 */
int hashRelation(char relation_name, int relation_size, Buffer *buf);

#endif //INITDATABLOCKS_H
