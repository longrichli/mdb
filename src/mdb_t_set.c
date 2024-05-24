#include "mdb.h"
#include "mdb_alloc.h"
#include "mdb_util.h"

// 集合相关命令
// SADD	向集合内添加元素。
// 例如：SADD key member1 member2
void mdbCommandSadd(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 3) {
        mdbSendReply(fd, "wrong number of arguments for 'sadd' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // 集合不存在，创建集合
        setObj = mdbCreateSetObject();
        mobj *key = mdbDupStringObject(c->argv[1]);
        // 将集合放入dict中
        mdbDictAdd(c->db->dict, key, setObj);
    }
    // 获取集合
    dict *set = setObj->ptr;
    for(int i = 2; i < c->argc; i++) {
        // 添加元素
        mobj *key = mdbDupStringObject(c->argv[i]);
        mobj *val = mdbDupStringObject(c->argv[i]);
        mdbDictAdd(set, key, val);
    }
    // 返回元素的数量
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", mdbDictSize(set));
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// SCARD	返回集合内元素的数量。
// 例如：SCARD key
void mdbCommandScard(mdbClient *c) {
    int fd = c->fd;
    if(c->argc!= 2) {
        mdbSendReply(fd, "wrong number of arguments for'scard' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // 集合不存在
        mdbSendReply(fd, "no such set\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取集合
    dict *set = setObj->ptr;
    // 返回元素的数量
    char buf[32];
    sprintf(buf, "%d\r\n", mdbDictSize(set));
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// SISMEMBER	查看指定元素是否存在于集合中。
// 例如：SISMEMBER key member
void mdbCommandSismember(mdbClient *c) {
    int fd = c->fd;
    if(c->argc!= 3) {
        mdbSendReply(fd, "wrong number of arguments for'sismember' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // 集合不存在
        mdbSendReply(fd, "no such set\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取集合
    dict *set = setObj->ptr;
    // 查看元素是否存在
    mobj *obj = mdbDictFetchValue(set, c->argv[2]);
    if(obj == NULL) {
        mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
    } else {
        mdbSendReply(fd, "1\r\n", MDB_REP_STRING);
    }

}
// SMEMBERS	返回集合内所有的元素。
// 例如：SMEMBERS key
void mdbCommandSmembers(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'smembers' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // set不存在
        mdbSendReply(fd, "ERR: no such set\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取set
    dict *set = setObj->ptr;
    // 获取键
    mobj **keys = (mobj **)mdbDictAllKey(set);
    // 构造回复
    SDS *reply = mdbSdsNewempty();
    for(int i = 0; i < mdbDictSize(set); i++) {
        reply = mdbSdscat(reply, ((SDS *)(keys[i]->ptr))->buf);
        reply = mdbSdscat(reply, "\r\n");
    }
    mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    mdbSdsfree(reply);
    mdbFree(keys);
}
// SRANDMEMBER	随机返回集合中的一个元素。
// 例如：SRANDMEMBER key
void mdbCommandSrandmember(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'smembers' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // set不存在
        mdbSendReply(fd, "ERR: no such set\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取set
    dict *set = setObj->ptr;
    // 获取键
    mobj **keys = (mobj **)mdbDictAllKey(set);
    int idx = rand() % mdbDictSize(set);
    mobj *retKey = mdbDupStringObject(keys[idx]);
    retKey->ptr = mdbSdscat((SDS *)retKey->ptr, "\r\n");
    mdbSendReply(fd, ((SDS *)retKey->ptr)->buf, MDB_REP_STRING);
    mdbFree(keys);
    mdbDecrRefCount(retKey);
}
// SPOP	随机返回并移除集合中一个或多个元素。
// 例如：SPOP key count
void mdbCommandSpop(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 2 || c->argc > 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'spop' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // set不存在
        mdbSendReply(fd, "ERR: no such set\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取set
    dict *set = setObj->ptr;
    long count = 1;
    if(c->argc == 3) {
        if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &count) < 0) {
            mdbSendReply(fd, "ERR: value is not an integer\r\n", MDB_REP_ERROR);
            return;
        }
        if(count < 1) {
            mdbSendReply(fd, "ERR: count is not greater than 0\r\n", MDB_REP_ERROR);
            return;
        }
        if(count > mdbDictSize(set)) {
            count = mdbDictSize(set);
        }
    }
    // 获取键
    mobj **keys = (mobj **)mdbDictAllKey(set);
    SDS *reply = mdbSdsNewempty();
    for(int i = 0; i < count; i++) {
        mobj *key = keys[i];
        reply = mdbSdscat(reply, ((SDS *)(key->ptr))->buf);
        reply = mdbSdscat(reply, "\r\n");
        mdbDictDelete(set, key);
    }
    mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    mdbSdsfree(reply);
    mdbFree(keys);
}
// SREM	移除集合中指定的元素。
// 例如：SREM key member1 member2
void mdbCommandSrem(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 3) {
        mdbSendReply(fd, "wrong number of arguments for 'zrem' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        mdbSendReply(fd, "no such set\r\n", MDB_REP_ERROR);
        return;
    }
    dict *set = setObj->ptr;
    int count = 0;
    for(int i = 2; i < c->argc; i++) {
       if(mdbDictFetchValue(set, c->argv[i]) != NULL) {
            mdbDictDelete(set, c->argv[i]);
            count++;
       }
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_OK);
}
