#include "mdb_skiplist.h"
#include "mdb_common.h"
#include "mdb_alloc.h"
#include "mdb_dict.h"
#include <math.h>
#define MDB_SKIPLIST_MAXLEVEL (32)
#define SKIPLIST_P (0.25)

skiplistNode *mdbSkipListCreateNode(int level, double score, mobj *obj) {
    skiplistNode *node = mdbMalloc(sizeof(*node) + level * sizeof(struct skiplistLevel));
    node->score = score;
    node->obj = obj;
    return node;
}

skiplist *mdbSkipListCreate(void) {
    int j;
    skiplist *sl;

    sl = mdbMalloc(sizeof(*sl));
    sl->level = 1;
    sl->length = 0;
    sl->header = mdbSkipListCreateNode(MDB_SKIPLIST_MAXLEVEL,0,NULL);
    for (j = 0; j < MDB_SKIPLIST_MAXLEVEL; j++) {
        sl->header->level[j].forward = NULL;
        sl->header->level[j].span = 0;
    }
    sl->header->backward = NULL;
    sl->tail = NULL;
    return sl;
}

void mdbSkipListFreeNode(skiplistNode *node) {
    mdbDecrRefCount(node->obj);
    mdbFree(node);
}

void mdbSkipListFree(skiplist *sl) {
    skiplistNode *node = sl->header->level[0].forward, *next;

    mdbFree(sl->header);
    while(node) {
        next = node->level[0].forward;
        mdbSkipListFreeNode(node);
        node = next;
    }
    mdbFree(sl);
}

int mdbSkipListRandomLevel(void) {
    int level = 1;
    while ((rand()&0xFFFF) < (SKIPLIST_P * 0xFFFF))
        level += 1;
    return (level<MDB_SKIPLIST_MAXLEVEL) ? level : MDB_SKIPLIST_MAXLEVEL;
}

skiplistNode *mdbSkipListInsert(skiplist *sl, double score, mobj *obj) {
    skiplistNode *update[MDB_SKIPLIST_MAXLEVEL], *x;
    unsigned int rank[MDB_SKIPLIST_MAXLEVEL];
    int i, level;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
      
        rank[i] = i == (sl->level-1) ? 0 : rank[i+1];
        while (x->level[i].forward &&
            (x->level[i].forward->score < score)) {
            rank[i] += x->level[i].span;
            x = x->level[i].forward;
        }
        update[i] = x;
    }
    level = mdbSkipListRandomLevel();
    if (level > sl->level) {
        for (i = sl->level; i < level; i++) {
            rank[i] = 0;
            update[i] = sl->header;
            update[i]->level[i].span = sl->length;
        }
        sl->level = level;
    }
    x = mdbSkipListCreateNode(level,score,obj);
    for (i = 0; i < level; i++) {
        x->level[i].forward = update[i]->level[i].forward;
        update[i]->level[i].forward = x;

        x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
        update[i]->level[i].span = (rank[0] - rank[i]) + 1;
    }

    for (i = level; i < sl->level; i++) {
        update[i]->level[i].span++;
    }

    x->backward = (update[0] == sl->header) ? NULL : update[0];
    if (x->level[0].forward)
        x->level[0].forward->backward = x;
    else
        sl->tail = x;
    sl->length++;
    return x;
}

void mdbSkipListDeleteNodeInternal(skiplist *sl, skiplistNode *x, skiplistNode **update) {
    int i;
    for (i = 0; i < sl->level; i++) {
        if (update[i]->level[i].forward == x) {
            update[i]->level[i].span += x->level[i].span - 1;
            update[i]->level[i].forward = x->level[i].forward;
        } else {
            update[i]->level[i].span -= 1;
        }
    }
    if (x->level[0].forward) {
        x->level[0].forward->backward = x->backward;
    } else {
        sl->tail = x->backward;
    }
    while(sl->level > 1 && sl->header->level[sl->level-1].forward == NULL)
        sl->level--;
    sl->length--;
}

int mdbSkipListDeleteNode(skiplist *sl, double score, mobj *obj) {
    skiplistNode *update[MDB_SKIPLIST_MAXLEVEL], *x;
    int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
            (x->level[i].forward->score < score))
            x = x->level[i].forward;
        update[i] = x;
    }
    x = x->level[0].forward;
    if (x && score == x->score && mdbCompareStringObjects(x->obj,obj) == 0) {
        mdbSkipListDeleteNodeInternal(sl, x, update);
        mdbSkipListFreeNode(x);
        return 1;
    } else {
        return 0; 
    }
    return 0; 
}

unsigned long mdbSkipListDeleteRangeByScore(skiplist *sl, rangespec range, dict *dict) {
    skiplistNode *update[MDB_SKIPLIST_MAXLEVEL], *x;
    unsigned long removed = 0;
    int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->level[i].forward && (range.minex ?
            x->level[i].forward->score <= range.min :
            x->level[i].forward->score < range.min))
                x = x->level[i].forward;
        update[i] = x;
    }

    x = x->level[0].forward;

    while (x && (range.maxex ? x->score < range.max : x->score <= range.max)) {
        skiplistNode *next = x->level[0].forward;
        mdbSkipListDeleteNodeInternal(sl,x,update);
        mdbDictDelete(dict,x->obj);
        mdbSkipListFreeNode(x);
        removed++;
        x = next;
    }
    return removed;
}

unsigned long mdbSkipListDeleteRangeByRank(skiplist *sl, unsigned int start, unsigned int end, dict *dict) {
    skiplistNode *update[MDB_SKIPLIST_MAXLEVEL], *x;
    unsigned long traversed = 0, removed = 0;
    int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->level[i].forward && (traversed + x->level[i].span) < start) {
            traversed += x->level[i].span;
            x = x->level[i].forward;
        }
        update[i] = x;
    }

    traversed++;
    x = x->level[0].forward;
    while (x && traversed <= end) {
        skiplistNode *next = x->level[0].forward;
        mdbSkipListDeleteNodeInternal(sl,x,update);
        mdbDictDelete(dict,x->obj);
        mdbSkipListFreeNode(x);
        removed++;
        traversed++;
        x = next;
    }
    return removed;
}

/* 返回大于等于给定分数的第一个节点.
 * 如果没有返回NULL */
skiplistNode *mdbSkipListFirstWithScore(skiplist *sl, double score) {
    skiplistNode *x;
    int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->level[i].forward && x->level[i].forward->score < score)
            x = x->level[i].forward;
    }
    return x->level[0].forward;
}

/* 根据分数和对象返回其排名.
 * 如果没有返回0
 */
unsigned long mdbSkipListGetRank(skiplist *sl, double score, mobj *o) {
    skiplistNode *x;
    unsigned long rank = 0;
    int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
            (x->level[i].forward->score < score ||
                (x->level[i].forward->score == score &&
                mdbCompareStringObjects(x->level[i].forward->obj,o) <= 0))) {
            rank += x->level[i].span;
            x = x->level[i].forward;
        }

        if (x->obj && mdbCompareStringObjects(x->obj,o) == 0) {
            return rank;
        }
    }
    return 0;
}

/* 通过排名寻找元素,排名从1开始 */
skiplistNode* mdbSkipListGetElementByBank(skiplist *sl, unsigned long rank) {
    skiplistNode *x;
    unsigned long traversed = 0;
    int i;

    x = sl->header;
    for (i = sl->level-1; i >= 0; i--) {
        while (x->level[i].forward && (traversed + x->level[i].span) <= rank)
        {
            traversed += x->level[i].span;
            x = x->level[i].forward;
        }
        if (traversed == rank) {
            return x;
        }
    }
    return NULL;
}


/*
des:    
    解析分数范围字符串
param:
    min: 分数范围最小值字符串
    max: 分数范围最大值字符串
    spec: 分数范围结构体
return:
    错误: -1
    成功: 0
*/
int mdbSkipListParseRange(char *min, char *max, rangespec *spec) {
    char *eptr;
    spec->minex = spec->maxex = 0;


        if (min[0] == '(') {
            spec->min = strtod((char*)min+1,&eptr);
            if (eptr[0] != '\0' || isnan(spec->min)) return -1;
            spec->minex = 1;
        } else {
            spec->min = strtod(min,&eptr);
            if (eptr[0] != '\0' || isnan(spec->min)) return -1;
        }
    
        if (max[0] == '(') {
            spec->max = strtod((char*)max+1,&eptr);
            if (eptr[0] != '\0' || isnan(spec->max)) return -1;
            spec->maxex = 1;
        } else {
            spec->max = strtod((char*)max,&eptr);
            if (eptr[0] != '\0' || isnan(spec->max)) return -1;
        }
    

    return 0;
}



