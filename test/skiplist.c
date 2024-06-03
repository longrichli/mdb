#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/mdb_skiplist.h"
#include "../src/mdb_sds.h"

int main(int argc, char **argv) {
    skiplist *sl = mdbSkipListCreate();
    mobj *aplle = mdbCreateStringObject("apple");
    mobj *orange = mdbCreateStringObject("orange");
    mdbSkipListInsert(sl, 4.2, aplle);
    mdbSkipListInsert(sl, 3.1, orange);
    skiplistNode *node = mdbSkipListGetElementByBank(sl, 1);
    printf("%s\n", ((SDS *)(node->obj->ptr))->buf);
    int rank = mdbSkipListGetRank(sl, 3.1, orange);
    printf("rank = %d\n", rank);
    skiplistNode *node1 = mdbSkipListFirstWithScore(sl, 4.0);
    printf("%s\n", ((SDS *)(node1->obj->ptr))->buf);
    
    return 0;
}