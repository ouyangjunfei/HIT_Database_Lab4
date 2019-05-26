/*
 * test.c
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#include <stdlib.h>
#include <stdio.h>

#include "initDataBlocks.h"
#include "select.h"
#include "projection.h"
#include "join.h"


int main(int argc, char **argv) {
    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */


    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }


    //createRelation(16, 'R', 1, 40, 1, 1000, &buf);
    //createRelation(32, 'S', 20, 60, 1, 1000, &buf);

    //32
    //sortRelationship('R','A',R_BLOCK,&buf);
    //64
    //sortRelationship('S', 'C', S_BLOCK, &buf);

    //64
    //externalSort('R', 'A', R_BLOCK, &buf);
    //128
    //externalSort('S', 'C', S_BLOCK, &buf);

    //18
    //LinearSearch('R', 'A', 40, &buf);
    //33
    //LinearSearch('S', 'C', 60, &buf);

    //12
    //BinarySearch('R', 'A', 40, &buf);
    //11
    //BinarySearch('S', 'C', 60, &buf);

    //204
    //printf("%d\n", hashRelation('R', R_BLOCK, &buf));
    //421
    //printf("%d\n", hashRelation('S', S_BLOCK, &buf));

    //3
    //HashSearch('R', 'A', 40, &buf);
    //2
    //HashSearch('S', 'C', 60, &buf);

    //24
    //Projection('R', 'A', &buf);
    //48
    //Projection('S', 'D', &buf);

    //196
    //NestLoopJoin('R', 'S', &buf);
    //759
    //SortMergeJoin('R','S',&buf);
    //108
    //HashJoin('R', 'S', &buf);
    //127
    Fixed_SortMergeJoin('R', 'S', &buf);


    freeBuffer(&buf);

    printf("\n");
    printf("# of IO's is %lu\n", buf.numIO); /* Check the number of IO's */

    return 0;
}

