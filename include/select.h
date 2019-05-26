//
// Created by Ouyangjunfei on 2019/5/14.
//

#ifndef SELECTALGORITHM_H
#define SELECTALGORITHM_H

#include "initDataBlocks.h"

/**
 * 按照关系属性值进行线性搜索
 * <br>R关系的查找结果存储磁盘块前缀为1000，S关系的查找结果存储磁盘块前缀为2000
 *
 * @param relation_name 关系名
 * @param field_name 属性名
 * @param number 被查找的属性值
 * @param buf 缓冲区指针
 */
void LinearSearch(char relation_name, char field_name, int number, Buffer *buf);

/**
 * 利用二分查找的搜索
 * <br>定位到被查找值的元组序号后,顺序向后输出直到元组的值不满足要求
 *
 * @param relation_name 关系名
 * @param field_name 属性名
 * @param number 被查找的属性值
 * @param buf 缓冲区指针
 */
void BinarySearch(char relation_name, char field_name, int number, Buffer *buf);

/**
 * 根据哈希桶的结构查找值
 *
 * @param relation_name 关系名
 * @param field_name 属性名
 * @param number 数值
 * @param buf 缓冲区指针
 */
void HashSearch(char relation_name, char field_name, int number, Buffer *buf);

#endif //SELECTALGORITHM_H
