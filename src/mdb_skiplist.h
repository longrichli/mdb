#ifndef __MDB_SKIPLIST_H__
#define __MDB_SKIPLIST_H__
#include "mdb_object.h"
#include "mdb_dict.h"

typedef struct skiplistNode {
    mobj *obj;
    double score;
    struct skiplistNode *backward;
    struct skiplistLevel {
        struct skiplistNode *forward;
        unsigned int span;
    } level[];
} skiplistNode;

typedef struct skiplist {
    struct skiplistNode *header, *tail;
    unsigned long length;
    int level;
} skiplist;


typedef struct {
    double min, max;
    int minex, maxex;
} rangespec;

typedef struct zset {
    skiplist *sl;
    dict *d;
} zset;

skiplist *mdbSkipListCreate(void);

void mdbSkipListFree(skiplist *sl);

skiplistNode *mdbSkipListInsert(skiplist *sl, double score, mobj *obj);

int mdbSkipListDeleteNode(skiplist *sl, double score, mobj *obj);

unsigned long mdbSkipListDeleteRangeByScore(skiplist *sl, rangespec range, dict *dict);

unsigned long mdbSkipListDeleteRangeByRank(skiplist *sl, unsigned int start, unsigned int end, dict *dict);

skiplistNode *mdbSkipListFirstWithScore(skiplist *sl, double score);

unsigned long mdbSkipListGetRank(skiplist *sl, double score, mobj *o);

skiplistNode* mdbSkipListGetElementByBank(skiplist *sl, unsigned long rank);

int mdbSkipListParseRange(char *min, char *max, rangespec *spec);



#endif /* __MDB_SKIPLIST_H__ */