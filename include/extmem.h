/*
 * extmem.h
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#ifndef EXTMEM_H
#define EXTMEM_H

#include <stddef.h>

#define BLOCK_AVAILABLE 0
#define BLOCK_UNAVAILABLE 1

typedef struct tagBuffer {
    unsigned long numIO; /* Number of IO's*/
    size_t bufSize; /* Buffer size*/
    size_t blkSize; /* Block size */
    size_t numAllBlk; /* Number of blocks that can be kept in the buffer */
    size_t numFreeBlk; /* Number of available blocks in the buffer */
    unsigned char *data; /* Starting address of the buffer */
} Buffer;

/**
 * Initialize a buffer with the specified buffer size and block size.
 * <br>If the initialization fails, the return value is NULL;
 * <br>otherwise the pointer to the buffer.
 *
 * @param bufSize 缓冲区大小(字节)
 * @param blkSize 块大小(字节)
 * @param buf 缓冲区指针(未初始化)
 * @return buf 缓冲区指针(分配地址)
 */
Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf);


/**
 * Free the memory used by a buffer.
 *
 * @param buf 缓冲区指针
 */
void freeBuffer(Buffer *buf);

/**
 * Apply for a new block from a buffer.
 * <br>If no free blocks are available in the buffer,
 * then the return value is NULL;
 * <br>otherwise the pointer to the block.
 *
 * @param buf 缓冲区指针
 * @return 新分配块的首地址
 */
unsigned char *getNewBlockInBuffer(Buffer *buf);

/**
 * Set a block in a buffer to be available.
 *
 * @param blk 待释放的块首地址
 * @param buf 缓冲区指针
 */
void freeBlockInBuffer(unsigned char *blk, Buffer *buf);

/**
 * Drop a block on the disk.
 *
 * @param addr 块的名称
 * @return 成功删除返回0，失败返回-1
 */
int dropBlockOnDisk(unsigned int addr);

/**
 * Read a block from the hard disk to the buffer by the address of the block.
 *
 * @param addr 被读取块的名称
 * @param buf 缓冲区指针
 * @return 块在缓冲区中的首地址
 */
unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf);

/**
 * Read a block in the buffer to the hard disk by the address of the block.
 *
 * @param blkPtr 块在缓冲区中的指针地址
 * @param addr 写出的块的名称
 * @param buf 缓冲区指针
 * @return 成功写出返回0，失败返回-1
 */
int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf);

#endif // EXTMEM_H
