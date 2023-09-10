#include "mdb.h"
#include "mdb_util.h"

// 字符串相关命令
// SET	向内存数据库中存入一个字符串。
// 例如：SET key value
void mdbCommandSet(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'set' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandSet() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    // 获取key
    mobj *key = mdbDupStringObject(c->argv[1]);
    if(key == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandSet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    // 获取value
    mobj *value = mdbDupStringObject(c->argv[2]);
    if(value == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandSet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    // 设置key-value
    if(mdbDictAdd(c->db->dict, key, value) < 0) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: set key-value failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandSet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    c->db->keynum++;
    // 设置成功, 发送ok
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandSet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        return;
    }
}
// GET	从内存数据库中取出一个字符串。
// 例如：GET key
void mdbCommandGet(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'get' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandGet() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    // 根据key获取value
    mobj *val = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(val == NULL) {
        // 发送nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_STRING) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandGet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    // 获取val的解码对象
    mobj *decodedVal = mdbGetDecodedObject(val);
    if(decodedVal == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandGet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    // 给val的值的后面加上'\r\n'
    mobj *valWithCRLF = mdbDupStringObject(decodedVal);
    if(valWithCRLF == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandGet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    valWithCRLF->ptr = mdbSdscat((SDS *)valWithCRLF->ptr, "\r\n");
    // 发送value
    if(mdbSendReply(fd, ((SDS *)valWithCRLF->ptr)->buf, MDB_REP_STRING) < 0) {
        mdbDecrRefCount(decodedVal);
        mdbDecrRefCount(valWithCRLF);
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandGet() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    mdbDecrRefCount(valWithCRLF);
    mdbDecrRefCount(decodedVal);
}
// APPEND	如果内存数据库存在某个字符串，对这个字符串进行追加，如果内存数据库不存在指定的字符串，则新存入一个字符串，字符串的内容为追加的内容。
// 例如：APPEND key value
void mdbCommandAppend(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'append' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandAppend() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取value
    mobj *val = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(val == NULL) {
        // 不存在key, 直接set
        mdbCommandSet(c);
        return;
    }
    // 获取val的解码对象
    mobj *decodedVal = mdbGetDecodedObject(val);
    if(decodedVal == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandAppend() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // dup 解码对象
    mobj *newVal = mdbDupStringObject(decodedVal);
    if(newVal == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandAppend() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 追加value
    newVal->ptr = mdbSdscat((SDS *)newVal->ptr, ((SDS *)c->argv[2]->ptr)->buf);
    // dup key
    mobj *key = mdbDupStringObject(c->argv[1]);
    // 设置key-value
    if(mdbDictReplace (c->db->dict, key, newVal) < 0) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: append key-value failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandAppend() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 释放内存
    mdbDecrRefCount(decodedVal);
    // 发送ok
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandAppend() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    } 
}
// INCRBY	如果字符串可以转换成整数，对其进行加法运算，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：INCRBY key increment
void mdbCommandIncrby(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'incrby' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    // 根据key获取value
    mobj *val = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(val == NULL) {
        // 不存在key, 看看value是否可以转换成整数
        long long increment;
        if(mdbIsStringRepresentableAsLong(c->argv[2]->ptr, &increment) < 0) {
            // 发送错误信息
            if(mdbSendReply(fd, "ERR: value is not an integer or out of range\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
            }
            goto __finish;
        }
        // 创建新的value
        mobj *newVal = mdbCreateStringObjectFromLongLong(increment);
        if(newVal == NULL) {
            // 发送错误信息
            if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
            }
            goto __finish;
        }
        // dup key
        mobj *key = mdbDupStringObject(c->argv[1]);
        // 设置key-value
        if(mdbDictAdd(c->db->dict, key, newVal) < 0) {
            // 发送错误信息
            if(mdbSendReply(fd, "ERR: add key-value failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
            }
            goto __finish;
        }
        // 回复OK
        if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    // 获取val的解码对象
    mobj *decodedVal = mdbGetDecodedObject(val);
    if(decodedVal == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    long long oldVal;
    if(mdbIsStringRepresentableAsLong(decodedVal->ptr, &oldVal) < 0) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: value is not an integer or out of range\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    // 看看value是否可以转换成整数
    long long increment;
    if(mdbIsObjectRepresentableAsLongLong(c->argv[2], &increment) < 0) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: value is not an integer or out of range\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    // 创建新的value
    mobj *newVal = mdbCreateStringObjectFromLongLong(increment + oldVal);
    if(newVal == NULL) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: out of memory\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    // dup key
    mobj *key = mdbDupStringObject(c->argv[1]);
    // 设置key-value
    if(mdbDictReplace (c->db->dict, key, newVal) < 0) {
        // 发送错误信息
        if(mdbSendReply(fd, "ERR: add key-value failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
        }
        goto __finish;
    }
    // 释放内存
    mdbDecrRefCount(decodedVal);
    // 发送ok
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandIncrby() | At %s:%d", __FILE__, __LINE__);
    }
__finish:
    return;
}
// DECRBY	如果字符串可以转换成整数，对其进行减法运算，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：DECRBY key decrement
void mdbCommandDecrby(mdbClient *c) {

}
// INCR	如果字符串可以转换成整数，对其进行加1，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：INCR key
void mdbCommandIncr(mdbClient *c) {

}
// STRLEN	返回指定字符串的长度。
// 例如：STRLEN key
void mdbCommandStrlen(mdbClient *c) {

}
// SETRANGE	指定一个偏移量，覆盖指定字符串偏移量后面的部分。
// 例如：SETRANGE key offset value
void mdbCommandSetrange(mdbClient *c) {

}
// GETRANGE	指定一个范围，返回字符串落在该范围内的字串。
// 例如：GETRANGE key start end
void mdbCommandGetrange(mdbClient *c) {

}