//
// Created by Ouyangjunfei on 2019/5/15.
//

#ifndef PROJECTION_H
#define PROJECTION_H

#include "initDataBlocks.h"

/**
 * 将关系的属性进行投影
 * <br>R关系结果投影前缀为10000,S关系投影前缀为20000
 *
 * @param relation_name
 * @param field_name
 * @param buf
 */
void Projection(char relation_name, char field_name, Buffer *buf);


#endif //PROJECTION_H
