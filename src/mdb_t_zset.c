#include "mdb.h"
#include "mdb_skiplist.h"
#include <math.h>
#include "mdb_util.h"
#include "mdb_sds.h"

// 有序集合相关命令
// ZADD	将元素和分值添加到有序集合中。
// 例如：ZADD key score1 member1 score2 member2
void mdbCommandZadd(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 4 || c->argc % 2 != 0) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zadd' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        // 创建一个新的有序集合对象
        zsetObj = mdbCreateZsetObject();
        mobj *newKey = mdbDupStringObject(c->argv[1]);
        mdbDictAdd(c->db->dict, newKey, zsetObj);
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    char *eptr;
    int count = 0;
    for(int i = 2; i < c->argc; i += 2) {
        double score = strtod(((SDS *)(c->argv[i]->ptr))->buf, &eptr);
        if (eptr[0] != '\0' || isnan(score)) {
            mdbSendReply(fd, "ERR: score is not a valid float\r\n", MDB_REP_ERROR);
            return;
        }
        mobj *memberObj = mdbDupStringObject(c->argv[i+1]);
        mobj *scoreObj = mdbDupStringObject(c->argv[i]);
        int r = mdbDictAdd(zset->d, memberObj, scoreObj);
        mdbIncrRefCount(memberObj);
        if(r < 0) {
            continue;
        }
        count++;
        mdbSkipListInsert(zset->sl, score, memberObj);
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// ZCARD	返回有序集合中元素的数量。
// 例如：ZCARD key
void mdbCommandZcard(mdbClient *c) {
    int fd = c->fd;
    if(c->argc!= 2) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zcard' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    char buf[32] = {0};
    sprintf(buf, "%lu\r\n", zset->sl->length);
    mdbSendReply(fd, buf, MDB_REP_STRING);

}
// ZCOUNT	返回分值在指定范围内的元素数量。
// 例如：ZCOUNT key min max
void mdbCommandZcount(mdbClient *c) {
    int fd = c->fd;
    if(c->argc!= 4) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zcount' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    char *eptr;
    double min = strtod(((SDS *)(c->argv[2]->ptr))->buf, &eptr);
    if (eptr[0] != '\0' || isnan(min)) {
        mdbSendReply(fd, "ERR: min is not a valid float\r\n", MDB_REP_ERROR);
        return;
    }
    double max = strtod(((SDS *)(c->argv[3]->ptr))->buf, &eptr);
    if (eptr[0] != '\0' || isnan(max)) {
        mdbSendReply(fd, "ERR: max is not a valid float\r\n", MDB_REP_ERROR);
        return;
    }
    if (min > max) {
            mdbSendReply(fd, "ERR: min is greater than max\r\n", MDB_REP_ERROR);
            return;
    }
    skiplistNode *node = mdbSkipListFirstWithScore(zset->sl, min);
    int count = 0;
    while(node != NULL && node->score <= max) {
        count++;
        node = node->level[0].forward;
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// ZRANGE	返回索引范围内的元素。
// 例如：ZRANGE key start stop
void mdbCommandZrange(mdbClient *c) {
    int fd = c->fd;
    int withscores = 0;
    if(c->argc != 4 && c->argc != 5) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zrange' command\r\n", MDB_REP_ERROR);
        return;
    }
    if(c->argc == 5 && !strcasecmp(((SDS *)(c->argv[4]->ptr))->buf, "WITHSCORES")) {
        withscores = 1;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_STRING);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
        // 获取start
    long start = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &start) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid start\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandZrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取stop
    long stop = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[3]->ptr), &stop) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid stop\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandZrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    int len = zset->sl->length;
    start = start < 0 ? len + start : start;
    stop = stop < 0 ? len + stop : stop;
    int tmp = max(start, stop);
    start = min(start, stop) + 1;
    stop = tmp + 1;
    mdbLogWrite(LOG_DEBUG, "mdbCommandZrange() start: %ld", start);
    mdbLogWrite(LOG_DEBUG, "mdbCommandZrange() stop: %ld", stop);
    // 获取结果集
    SDS *result = mdbSdsNewempty();
    skiplistNode *node = mdbSkipListGetElementByBank(zset->sl, start);
    for(int i = start; i <= stop && node != NULL; i++) {
        mobj *memberObj = node->obj;
        result = mdbSdscat(result, ((SDS *)(memberObj->ptr))->buf);
        result = mdbSdscat(result, "\r\n");
        if(withscores) {
            char buf[64] = {0};
            sprintf(buf, "%lf", node->score);
            result = mdbSdscat(result, buf);
            result = mdbSdscat(result, "\r\n");
        }
        node = node->level[0].forward;
    }
    if(result->len == 0) {
        mdbSendReply(fd, "(empty set)\r\n", MDB_REP_NIL);
    } else {
        mdbSendReply(fd, result->buf, MDB_REP_STRING);
    }
    mdbSdsfree(result);
}
// ZREVRANGE	按分值从大到小返回索引范围内的元素。
// 例如：ZREVRANGE key start stop
void mdbCommandZrevrange(mdbClient *c) {
    int fd = c->fd;
    int withscores = 0;
    if(c->argc != 4 && c->argc != 5) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zrevrange' command\r\n", MDB_REP_ERROR);
        return;
    }
    if(c->argc == 5 && !strcasecmp(((SDS *)(c->argv[4]->ptr))->buf, "WITHSCORES")) {
        withscores = 1;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_STRING);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
        // 获取start
    long start = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &start) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid start\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandZrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取stop
    long stop = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[3]->ptr), &stop) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid stop\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandZrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    int len = zset->sl->length;
    start = start < 0 ? len + start : start;
    stop = stop < 0 ? len + stop : stop;
    int tmp = max(start, stop);
    start = min(start, stop) + 1;
    stop = tmp + 1;
    mdbLogWrite(LOG_DEBUG, "mdbCommandZrange() start: %ld", start);
    mdbLogWrite(LOG_DEBUG, "mdbCommandZrange() stop: %ld", stop);
    // 获取结果集
    SDS *result = mdbSdsNewempty();
    skiplistNode *node = mdbSkipListGetElementByBank(zset->sl, stop);
    for(int i = stop; i >= start && node != zset->sl->header; i--) {
        mobj *memberObj = node->obj;
        result = mdbSdscat(result, ((SDS *)(memberObj->ptr))->buf);
        result = mdbSdscat(result, "\r\n");
        if(withscores) {
            char buf[64] = {0};
            sprintf(buf, "%lf", node->score);
            result = mdbSdscat(result, buf);
            result = mdbSdscat(result, "\r\n");
        }
        node = node->backward;
    }
    if(result->len == 0) {
        mdbSendReply(fd, "(empty set)\r\n", MDB_REP_NIL);
    } else {
        mdbSendReply(fd, result->buf, MDB_REP_STRING);
    }
    mdbSdsfree(result);
}
// ZRANK	返回元素的排名。
// 例如：ZRANK key member
void mdbCommandZrank(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zrank' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    mobj *memberSocre = mdbDictFetchValue(zset->d, c->argv[2]);
    if(memberSocre == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    double score = strtod(((SDS *)(memberSocre->ptr))->buf, NULL);
    unsigned long rank = mdbSkipListGetRank(zset->sl, score, c->argv[2]);
    char buf[32] = {0};
    sprintf(buf, "%lu\r\n", rank);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// ZREVRANK	返回元素的反向排名。
// 例如：ZREVRANK key member
void mdbCommandZrevrank(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zrevrank' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    mobj *memberSocre = mdbDictFetchValue(zset->d, c->argv[2]);
    if(memberSocre == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    double score = strtod(((SDS *)(memberSocre->ptr))->buf, NULL);
    unsigned long rank = mdbSkipListGetRank(zset->sl, score, c->argv[2]);
    unsigned long len = zset->sl->length;
    unsigned long revRank = len - rank + 1;
    char buf[32] = {0};
    sprintf(buf, "%lu\r\n", revRank);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// ZREM	删除给定的元素及其分值。
// 例如：ZREM key member1 member2
void mdbCommandZrem(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zrem' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    int count = 0;
    for(int i = 2; i < c->argc; i++) {
        mobj *memberSocre = mdbDictFetchValue(zset->d, c->argv[i]);
        if(memberSocre == NULL) {
            continue;
        }
        double score = strtod(((SDS *)(memberSocre->ptr))->buf, NULL);
        mdbSkipListDeleteNode(zset->sl, score, c->argv[i]);
        mdbDictDelete(zset->d, c->argv[i]);
        count++;
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// ZSCORE	返回指定元素的分值。
// 例如：ZSCORE key member
void mdbCommandZscore(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'zscore' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj *zsetObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(zsetObj == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    if(zsetObj->type != MDB_ZSET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    zset *zset = zsetObj->ptr;
    mobj *memberSocre = mdbDictFetchValue(zset->d, c->argv[2]);
    if(memberSocre == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    double score = strtod(((SDS *)(memberSocre->ptr))->buf, NULL);
    char buf[64] = {0};
    sprintf(buf, "%lf\r\n", score);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}