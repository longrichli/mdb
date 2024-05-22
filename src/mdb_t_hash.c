#include "mdb.h"
#include "mdb_alloc.h"

// 哈希相关命令
// HSET	向内存数据库中添加键值对。
// 例如：HSET key field value
void mdbCommandHset(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 4 || c->argc % 2 != 0) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'hset' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        //创建hash表
        hashObj = mdbCreateHashObject();
        mobj *key = mdbDupStringObject(c->argv[1]);
        mdbDictAdd(c->db->dict, key, hashObj);
    }
    // 获取hash表
    dict *hash = hashObj->ptr;
    // 添加键值对
    int count = 0;
    for(int i = 2; i < c->argc; i +=2 ) {
        mobj *key = mdbDupStringObject(c->argv[i]);
        mobj *val = mdbDupStringObject(c->argv[i+1]);
        if(mdbDictFetchValue(hash, key) == NULL) {
            count++;
        }
        mdbDictReplace(hash, key, val);
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_STRING);

}
// HGET	输入键获取值。
// 例如：HGET key field
void mdbCommandHget(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'hget' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        // hash表不存在
        mdbSendReply(fd, "ERR: no such hash\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取hash表
    dict *hash = hashObj->ptr;
    // 获取值
    mobj *val = mdbDictFetchValue(hash, c->argv[2]);
    if(val == NULL) {
        mdbSendReply(fd, "nil\r\n", MDB_REP_NIL);
        return;
    }
    mobj *retVal = mdbDupStringObject(val);
    SDS *ret = mdbSdscat(((SDS *)(retVal->ptr)), "\r\n");
    mdbSendReply(fd, ret->buf, MDB_REP_STRING);

}
// HEXSITS	查看一个键是否存在。
// 例如：HEXSITS key field
void mdbCommandHexists(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'hget' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        // hash表不存在
        mdbSendReply(fd, "ERR: no such hash\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取hash表
    dict *hash = hashObj->ptr;
    // 获取值
    mobj *val = mdbDictFetchValue(hash, c->argv[2]);
    if(val == NULL) {
        mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
        return;
    }
    mdbSendReply(fd, "1\r\n", MDB_REP_STRING);
}
// HDEL	根据键删除该键所在的键值对。
// 例如：HDEL key field
void mdbCommandHdel(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 3) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'hdel' command\r\n", MDB_REP_STRING);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        // hash表不存在
        mdbSendReply(fd, "ERR: no such hash\r\n", MDB_REP_STRING);
        return;
    }
    // 获取hash表
    dict *hash = hashObj->ptr;
    // 删除键值对
    int count = 0;
    for(int i = 2; i < c->argc; i++) {
        if(mdbDictFetchValue(hash, c->argv[i]) != NULL) {
            mdbDictDelete(hash, c->argv[i]);
            count++;
        }
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// HLEN	返回哈希表中键值对的数量。
// 例如：HLEN key
void mdbCommandHlen(mdbClient *c) {
    int fd = c->fd;
    if(c->argc!= 2) {
        mdbSendReply(fd, "ERR: wrong number of arguments for'hlen' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        // hash不存在
        mdbSendReply(fd, "ERR: no such hash\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取集合
    dict *hash = hashObj->ptr;
    // 返回元素的数量
    char buf[32];
    sprintf(buf, "%d\r\n", mdbDictSize(hash));
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// HKEYS	返回哈希表中的所有键。
// 例如：HKEYS key
void mdbCommandHkeys(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'hkeys' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        // hash不存在
        mdbSendReply(fd, "ERR: no such hash\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取哈希
    dict *hash = hashObj->ptr;
    // 获取键
    mobj **keys = (mobj **)mdbDictAllKey(hash);
    // 构造回复
    SDS *reply = mdbSdsNewempty();
    for(int i = 0; i < mdbDictSize(hash); i++) {
        mdbSdscat(reply, ((SDS *)(keys[i]->ptr))->buf);
        mdbSdscat(reply, "\r\n");
    }
    mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    mdbSdsfree(reply);
    mdbFree(keys);

}
// HGETALL	返回哈希表中所有的键值对。
// 例如：HGETALL key
void mdbCommandHgetall(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        mdbSendReply(fd, "ERR: wrong number of arguments for 'hgetall' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *hashObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(hashObj == NULL) {
        // hash不存在
        mdbSendReply(fd, "ERR: no such hash\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取哈希
    dict *hash = hashObj->ptr;
    // 获取键值对
    mobj **keys = (mobj **)mdbDictAllKey(hash);
    // 构造回复
    SDS *reply = mdbSdsNewempty();
    for(int i = 0; i < mdbDictSize(hash); i++) {
        mobj *key = keys[i];
        mobj *val = mdbDictFetchValue(hash, key);
        mdbLogWrite(LOG_DEBUG, "key: %s, val: %s\n", ((SDS *)(key->ptr))->buf, ((SDS *)(val->ptr))->buf);
        mdbSdscat(reply, ((SDS *)(key->ptr))->buf);
        mdbSdscat(reply, "\r\n");
        mdbSdscat(reply, ((SDS *)(val->ptr))->buf);
        mdbSdscat(reply, "\r\n");
    }
    mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    mdbSdsfree(reply);
    mdbFree(keys);
}