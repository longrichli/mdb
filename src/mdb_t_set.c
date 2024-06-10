#include "mdb.h"
#include "mdb_alloc.h"
#include "mdb_util.h"
#include "mdb_intset.h"
static mobj *mdbIntsetToDict(mdbClient *c, mobj *setObj) {
    mobj *dSetObj = NULL;
    intset *iset = NULL;
    dict *set = NULL;
    uint32_t len = 0;
    long val = 0;
    dSetObj = mdbCreateSetObject();
    iset = setObj->ptr;
    set = dSetObj->ptr;
    len = mdbIntsetLen(iset);
    for(uint32_t i = 0; i < len; i++) {
        mdbIntsetGet(iset, i, &val);
        mobj *intkey = mdbCreateStringObjectFromLongLong(val);
        mobj *intval = intkey;
        mdbIncrRefCount(intkey);
        mdbDictAdd(set, intval, intkey);
    }
    mdbDictDelete(c->db->dict, c->argv[1]);
    mdbIncrRefCount(c->argv[1]);
    mdbDictAdd(c->db->dict, c->argv[1], dSetObj);
    return dSetObj;
}
// 集合相关命令
// SADD	向集合内添加元素。
// 例如：SADD key member1 member2
void mdbCommandSadd(mdbClient *c) {
    int fd = c->fd;
    intset *iset = NULL;
    dict *set = NULL;
    if(c->argc < 3) {
        mdbSendReply(fd, "wrong number of arguments for 'sadd' command\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据key查找值
    mobj *setObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(setObj == NULL) {
        // 集合不存在，首先创建整数集合
        // setObj = mdbCreateSetObject();
        setObj = mdbCreateIntsetObject();
        mobj *key = mdbDupStringObject(c->argv[1]);
        // 将集合放入dict中
        mdbDictAdd(c->db->dict, key, setObj);
    }
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    // 获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else if(setObj->encoding == MDB_ENCODING_HT) {
        set = setObj->ptr;
    }
    for(int i = 2; i < c->argc; i++) {
        // 添加元素
        mobj *key = mdbDupStringObject(c->argv[i]);
        long integer = 0;
        if(set == NULL && mdbIsStringRepresentableAsLong(key->ptr, &integer) < 0) {
            // 将整数集合转成字典
            setObj = mdbIntsetToDict(c, setObj);
            set = setObj->ptr;
            iset = NULL;
        }
        
        mobj *val = key;
        mdbIncrRefCount(key);
        if(set == NULL) {
            iset = mdbIntsetAdd(iset, integer, NULL);
        } else {
            mdbDictAdd(set, key, val);
        }
        
    }
    // 返回元素的数量
    char buf[32] = {0};
    if(set == NULL) {
        sprintf(buf, "%d\r\n", mdbIntsetLen(iset));
    } else {
        sprintf(buf, "%d\r\n", mdbDictSize(set));
    }
    mdbSendReply(fd, buf, MDB_REP_STRING);
    // AOF 追加
    mdbAppendAOF(c);
}
// SCARD	返回集合内元素的数量。
// 例如：SCARD key
void mdbCommandScard(mdbClient *c) {
    int fd = c->fd;
    dict *set = NULL;
    intset *iset = NULL;
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
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据编码获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else {
        set = setObj->ptr;
    }
    
    // 返回元素的数量
    char buf[32] = {0};
    if(set == NULL) {
        sprintf(buf, "%d\r\n", mdbIntsetLen(iset));
    } else {
        sprintf(buf, "%d\r\n", mdbDictSize(set));
    }
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// SISMEMBER	查看指定元素是否存在于集合中。
// 例如：SISMEMBER key member
void mdbCommandSismember(mdbClient *c) {
    intset *iset = NULL;
    dict *set = NULL;
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
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
     // 根据编码获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else {
        set = setObj->ptr;
    }
    if(set == NULL) {
        long val = 0;
        if(mdbIsStringRepresentableAsLong(((SDS *)(c->argv[2]->ptr)), &val) < 0) {
            mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
            return;
        }
        if(mdbIntsetFind(iset, val)) {
            mdbSendReply(fd, "1\r\n", MDB_REP_STRING);
        } else {
            mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
        }
    } else {
        // 查看元素是否存在
        mobj *obj = mdbDictFetchValue(set, c->argv[2]);
        if(obj == NULL) {
            mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
        } else {
            mdbSendReply(fd, "1\r\n", MDB_REP_STRING);
        }
    }
    
}
// SMEMBERS	返回集合内所有的元素。
// 例如：SMEMBERS key
void mdbCommandSmembers(mdbClient *c) {
    int fd = c->fd;
    intset *iset = NULL;
    dict *set = NULL;
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
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
     // 根据编码获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else {
        set = setObj->ptr;
    }
    // 构造回复
    SDS *reply = mdbSdsNewempty();
    if(set == NULL) {
        for(uint32_t i = 0; i < mdbIntsetLen(iset); i++) {
            long val = 0;
            mdbIntsetGet(iset, i, &val);
            char buf[32] = {0};
            sprintf(buf, "%ld\r\n", val);
            reply = mdbSdscat(reply, buf);
        }
    } else {
        // 获取键
        mobj **keys = (mobj **)mdbDictAllKey(set);  
        for(int i = 0; i < mdbDictSize(set); i++) {
            mobj *key = mdbGetDecodedObject(keys[i]);
            reply = mdbSdscat(reply, ((SDS *)(key->ptr))->buf);
            reply = mdbSdscat(reply, "\r\n");
            mdbDecrRefCount(key);
        }
        mdbFree(keys);
    }
    mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    mdbSdsfree(reply);
    
}
// SRANDMEMBER	随机返回集合中的一个元素。
// 例如：SRANDMEMBER key
void mdbCommandSrandmember(mdbClient *c) {
    int fd = c->fd;
    intset *iset = NULL;
    dict *set = NULL;
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
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据编码获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else {
        set = setObj->ptr;
    }
    int idx = 0;
    mobj *retKey = NULL;
    if(set == NULL) {
        idx = rand() % mdbIntsetLen(iset);
        long val = 0;
        mdbIntsetGet(iset, idx, &val);
        char buf[32] = {0};
        sprintf(buf, "%ld\r\n", val);
        retKey = mdbCreateStringObject(buf);
    } else {
        // 获取键
        mobj **keys = (mobj **)mdbDictAllKey(set);
        idx = rand() % mdbDictSize(set);
        mobj *key = mdbGetDecodedObject(keys[idx]);
        retKey = mdbDupStringObject(key);
        retKey->ptr = mdbSdscat((SDS *)retKey->ptr, "\r\n");
        mdbFree(keys);
        mdbDecrRefCount(key);
    }
    
    mdbSendReply(fd, ((SDS *)retKey->ptr)->buf, MDB_REP_STRING);
    mdbDecrRefCount(retKey);
}
// SPOP	随机返回并移除集合中一个或多个元素。
// 例如：SPOP key count
void mdbCommandSpop(mdbClient *c) {
    intset *iset = NULL;
    dict *set = NULL;
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
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
    // 根据编码获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else {
        set = setObj->ptr;
    }
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
        if(set == NULL) {
            if(count > mdbIntsetLen(iset)) {
                count = mdbIntsetLen(iset);
            }
        } else {
            if(count > mdbDictSize(set)) {
                count = mdbDictSize(set);
            }
        } 
    }

    SDS *reply = mdbSdsNewempty();
    if(set == NULL) {
        for(uint32_t i = 0; i < count; i++) {
            long val = 0;
            mdbIntsetGet(iset, 0, &val);
            char buf[32] = {0};
            sprintf(buf, "%ld\r\n", val);
            reply = mdbSdscat(reply, buf);
            mdbIntsetRemvoe(iset, val, NULL);
        }
    } else {
        // 获取键
        mobj **keys = (mobj **)mdbDictAllKey(set);
        for(int i = 0; i < count; i++) {
            mobj *key = mdbGetDecodedObject(keys[i]);
            reply = mdbSdscat(reply, ((SDS *)(key->ptr))->buf);
            reply = mdbSdscat(reply, "\r\n");
            mdbDictDelete(set, keys[i]);
            mdbDecrRefCount(key);
        }
        mdbFree(keys);
    }
    
    mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    mdbSdsfree(reply);
    // AOF 追加
    mdbAppendAOF(c);
}
// SREM	移除集合中指定的元素。
// 例如：SREM key member1 member2
void mdbCommandSrem(mdbClient *c) {
    intset *iset = NULL;
    dict *set = NULL;
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
    // 检查对象的类型是否正确
    if(setObj->type!= MDB_SET) {
        mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR);
        return;
    }
     // 根据编码获取集合
    if(setObj->encoding == MDB_ENCODING_INTSET) {
        iset = setObj->ptr;
    } else {
        set = setObj->ptr;
    }
    int count = 0;
    if(set == NULL) {
        for(int i = 2; i < c->argc; i++) {
            int success = 0;
            long val = 0;
            mdbIsStringRepresentableAsLong(((SDS *)c->argv[i]->ptr), &val);
            mdbIntsetRemvoe(iset, val, &success);
            if(success) {
                count++;
            }
        }
    } else {
        for(int i = 2; i < c->argc; i++) {
            if(mdbDictFetchValue(set, c->argv[i]) != NULL) {
                    mdbDictDelete(set, c->argv[i]);
                    count++;
            }
        }
    }
    
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_OK);
    // AOF 追加
    mdbAppendAOF(c);
}
