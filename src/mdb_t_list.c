#include "mdb.h"
#include "mdb_list.h"
#include "mdb_sds.h"
#include "mdb_util.h"



// 列表相关命令
// LPUSH	添加元素到列表的表头。
// 例如：LPUSH key value1 value2
void mdbCommandLpush(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 3) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'lpush' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象

    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        // 不存在则创建一个新的列表对象
        listObj = mdbCreateListObject();
        if(listObj == NULL) {
            // 创建失败
            if(mdbSendReply(fd, "ERR: create list object failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandLpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
        // 添加到数据库字典
        mobj *listKey = mdbDupStringObject(c->argv[1]);
        if(mdbDictAdd(c->db->dict, listKey, listObj) < 0) {
            // 添加失败
            if(mdbSendReply(fd, "ERR: add list object to dict failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandLpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    for(int i = 2; i < c->argc; i++) {
        // 创建一个新的字符串对象
        mobj *obj = mdbDupStringObject(c->argv[i]);
        if(obj == NULL) {
            // 创建失败
            if(mdbSendReply(fd, "ERR: create string object failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandLpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }


#ifdef TEST
    // 看看obj的值
    mdbLogWrite(LOG_DEBUG, "obj: %s", obj == NULL ? "NULL": "NOT NULL");
    mdbLogWrite(LOG_DEBUG, "obj str: %s", ((SDS *)(obj->ptr))->buf);
    mdbLogWrite(LOG_DEBUG, "obj len: %d", ((SDS *)(obj->ptr))->len);
#endif
        // 添加到列表
        if(mdbListAddNodeHead(l, obj) == NULL) {
            // 添加失败
            if(mdbSendReply(fd, "ERR: add node to list failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandLpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
    }
#ifdef TEST
    // 遍历列表，看添加进去了没
    listNode *node = mdbListFirst(l);
    while(node != NULL) {
        mobj *obj = node->value;
        mdbLogWrite(LOG_DEBUG, "obj: %s", obj == NULL ? "NULL": "NOT NULL");
        mdbLogWrite(LOG_DEBUG, "obj str: %s", ((SDS *)(obj->ptr))->buf);
        mdbLogWrite(LOG_DEBUG, "obj len: %d", ((SDS *)(obj->ptr))->len);
        node = mdbListNextNode(node);
    }
#endif
    // 回复客户端OK
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // AOF 追加
    mdbAppendAOF(c);
}
// RPUSH	添加元素到列表的表尾。
// 例如：RPUSH key value1 value2
void mdbCommandRpush(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 3) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'rpush' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        // 不存在则创建一个新的列表对象
        listObj = mdbCreateListObject();
        if(listObj == NULL) {
            // 创建失败
            if(mdbSendReply(fd, "ERR: create list object failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
        mobj *listKey = mdbDupStringObject(c->argv[1]);
        // 添加到数据库字典
        if(mdbDictAdd(c->db->dict,listKey, listObj) < 0) {
            // 添加失败
            if(mdbSendReply(fd, "ERR: add list object to dict failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    for(int i = 2; i < c->argc; i++) {
        // 创建一个新的字符串对象
        mobj *obj = mdbDupStringObject(c->argv[i]);
        if(obj == NULL) {
            // 创建失败
            if(mdbSendReply(fd, "ERR: create string object failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
        // 添加到列表
        if(mdbListAddNodeTail(l, obj) == NULL) {
            // 添加失败
            if(mdbSendReply(fd, "ERR: add node to list failed\r\n", MDB_REP_ERROR) < 0) {
                // 发送失败
                mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            }
            return;
        }
    }
#ifdef TEST
    // 遍历列表，看添加进去了没
    listNode *node = mdbListFirst(l);
    while(node != NULL) {
        mobj *obj = node->value;
        mdbLogWrite(LOG_DEBUG, "obj: %s", obj == NULL ? "NULL": "NOT NULL");
        mdbLogWrite(LOG_DEBUG, "obj str: %s", ((SDS *)(obj->ptr))->buf);
        mdbLogWrite(LOG_DEBUG, "obj len: %d", ((SDS *)(obj->ptr))->len);
        node = mdbListNextNode(node);
    }
#endif
    // 回复客户端OK
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // AOF 追加
    mdbAppendAOF(c);
}
// LPOP	弹出列表的表头节点。
// 例如：LPOP key
void mdbCommandLpop(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'lpop' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        mdbLogWrite(LOG_WARNING, "mdbCommandLpop() key not exists | At %s:%d", __FILE__, __LINE__);
        // 不存在则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLpop() list length: %d", l->len);
    // 弹出表头节点
    listNode *node = mdbListFirst(l);
    if(node == NULL || l->len == 0) {
        
        // mdbDecrRefCount(listObj);
        // 为空则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取节点的字符串对象
    mobj *obj = node->value;
    mdbLogWrite(LOG_DEBUG, "obj: %s", obj == NULL ? "NULL": "NOT NULL");
    // dup一个新的字符串对象
    mobj *newObj = mdbDupStringObject(obj);
    mdbLogWrite(LOG_DEBUG, "newObj: %s", newObj == NULL ? "NULL": "NOT NULL");

    if(newObj == NULL) {
        // 创建失败
        if(mdbSendReply(fd, "ERR: create string object failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "newobj str: %s", ((SDS *)(newObj->ptr))->buf);
    mdbLogWrite(LOG_DEBUG, "newobj len: %d", ((SDS *)(newObj->ptr))->len);
    // 添加\r\n
    newObj->ptr = mdbSdscat((SDS *)newObj->ptr, "\r\n");
    mdbLogWrite(LOG_DEBUG, "newobj str: %s", ((SDS *)(newObj->ptr))->buf);
    // 回复客户端字符串对象
    if(mdbSendReply(fd, ((SDS *)(newObj->ptr))->buf, MDB_REP_STRING) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // 释放新的字符串对象
    mdbDecrRefCount(newObj);
    // 释放节点
    mdbListDelNode(l, node);
    // AOF 追加
    mdbAppendAOF(c);
#ifdef TEST
    // 看看链表头是否为空
    node = mdbListFirst(l);
    if(node == NULL) {
        mdbLogWrite(LOG_DEBUG, "mdbCommandLpop() list is empty | At %s:%d", __FILE__, __LINE__);
    }
#endif
}
// RPOP	弹出列表的表尾节点。
// 例如：RPOP key
void mdbCommandRpop(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'lpop' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        mdbLogWrite(LOG_WARNING, "mdbCommandLpop() key not exists | At %s:%d", __FILE__, __LINE__);
        // 不存在则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLpop() list length: %d", l->len);
    // 弹出表尾节点
    listNode *node = mdbListLast(l);
    if(node == NULL || l->len == 0) {
        
        // mdbDecrRefCount(listObj);
        // 为空则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取节点的字符串对象
    mobj *obj = node->value;
    mdbLogWrite(LOG_DEBUG, "obj: %s", obj == NULL ? "NULL": "NOT NULL");
    // dup一个新的字符串对象
    mobj *newObj = mdbDupStringObject(obj);
    mdbLogWrite(LOG_DEBUG, "newObj: %s", newObj == NULL ? "NULL": "NOT NULL");

    if(newObj == NULL) {
        // 创建失败
        if(mdbSendReply(fd, "ERR: create string object failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "newobj str: %s", ((SDS *)(newObj->ptr))->buf);
    mdbLogWrite(LOG_DEBUG, "newobj len: %d", ((SDS *)(newObj->ptr))->len);
    // 添加\r\n
    newObj->ptr = mdbSdscat((SDS *)newObj->ptr, "\r\n");
    mdbLogWrite(LOG_DEBUG, "newobj str: %s", ((SDS *)(newObj->ptr))->buf);
    // 回复客户端字符串对象
    if(mdbSendReply(fd, ((SDS *)(newObj->ptr))->buf, MDB_REP_STRING) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLpop() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // 释放新的字符串对象
    mdbDecrRefCount(newObj);
    // 释放节点
    mdbListDelNode(l, node);
    // AOF 追加
    mdbAppendAOF(c);
#ifdef TEST
    // 看看链表头是否为空
    node = mdbListFirst(l);
    if(node == NULL) {
        mdbLogWrite(LOG_DEBUG, "mdbCommandLpop() list is empty | At %s:%d", __FILE__, __LINE__);
    }
#endif
}
// LINDEX	返回列表中指定索引的节点。
// 例如：LINDEX key index
void mdbCommandLindex(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'lindex' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLindex() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        mdbLogWrite(LOG_WARNING, "mdbCommandLindex() key not exists | At %s:%d", __FILE__, __LINE__);
        // 不存在则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLindex() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLindex() list length: %d", l->len);
    // 获取索引
    long index = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &index) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid index\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLindex() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "mdbCommandLindex() index: %ld", index);
    // 获取节点
    listNode *node = mdbListIndex(l, index);
    if(node == NULL) {
        // 为空则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLindex() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取节点的字符串对象
    mobj *obj = node->value;
    mdbLogWrite(LOG_DEBUG, "obj: %s", obj == NULL ? "NULL": "NOT NULL");
    // dup一个新的字符串对象
    mobj *newObj = mdbDupStringObject(obj);
    mdbLogWrite(LOG_DEBUG, "newObj: %s", newObj == NULL ? "NULL": "NOT NULL");
    // 添加\r\n
    newObj->ptr = mdbSdscat((SDS *)newObj->ptr, "\r\n");
    mdbLogWrite(LOG_DEBUG, "newobj str: %s", ((SDS *)(newObj->ptr))->buf);
    // 回复客户端字符串对象
    if(mdbSendReply(fd, ((SDS *)(newObj->ptr))->buf, MDB_REP_STRING) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLindex() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // 释放新的字符串对象
    mdbDecrRefCount(newObj);
}
// LLEN	返回列表的长度。
// 例如：LLEN key
void mdbCommandLlen(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'llen' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLlen() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    mdbLogWrite(LOG_DEBUG, "mdbCommandLlen() key: %s", ((SDS *)(c->argv[1]->ptr))->buf);
    if(listObj == NULL) {
        mdbLogWrite(LOG_WARNING, "mdbCommandLlen() key not exists | At %s:%d", __FILE__, __LINE__);
        // 不存在则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLlen() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLlen() list length: %d", l->len);
    // 回复客户端列表长度
    char buf[32];
    int len = sprintf(buf, "%zu\r\n", l->len);
    if(mdbSendReply(fd, buf, len) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLlen() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
}
// LINSERT	在列表中指定元素的前面或后面添加新元素，如果列表中不存在指定元素，列表保持不变。
// 例如：LINSERT key BEFORE pivot value
void mdbCommandLinsert(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 5) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'linsert' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        mdbLogWrite(LOG_WARNING, "mdbCommandLinsert() key not exists | At %s:%d", __FILE__, __LINE__);
        // 不存在则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() list length: %d", l->len);
    // 获取pivot
    listNode *pivotNode = mdbListSearchKey(l, c->argv[3]);
    if(pivotNode == NULL) {
        // 创建失败
        if(mdbSendReply(fd, "ERR: create string object failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mobj *pivotObj = (mobj *)(pivotNode->value);
    // 获取value
    mobj *valueObj = mdbDupStringObject(c->argv[4]);
    if(valueObj == NULL) {
        // 创建失败
        if(mdbSendReply(fd, "ERR: create string object failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取方向
    int after = 0;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() direction: %s", ((SDS *)(c->argv[2]->ptr))->buf);
    if(strcasecmp(((SDS *)(c->argv[2]->ptr))->buf, "before") == 0) {
        after = 0;
    } else if(strcasecmp(((SDS *)(c->argv[2]->ptr))->buf, "after") == 0) {
        after = 1;
    } else {
        // 方向错误
        if(mdbSendReply(fd, "ERR: invalid direction\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() pivot: %s", ((SDS *)(pivotObj->ptr))->buf);
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() value: %s", ((SDS *)(valueObj->ptr))->buf);
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() after: %d", after);
    // 获取节点
    listNode *node = mdbListSearchKey(l, pivotObj);
    if(node == NULL) {
        // 为空则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() node: %s", node == NULL ? "NULL": "NOT NULL");
    // 添加到节点前或后
    l = mdbListInsertNode(l, node, valueObj, after);
    mdbLogWrite(LOG_DEBUG, "mdbCommandLinsert() list: %s", l == NULL ? "NULL": "NOT NULL");
    if(l == NULL) {
        // 添加失败
        if(mdbSendReply(fd, "ERR: insert node to list failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    
    }
    // 发送ok
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLinsert() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // AOF 追加
    mdbAppendAOF(c);
}
// LREM	删除列表中指定的节点。
// 例如：LREM key count value
void mdbCommandLrem(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 4) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'lrem' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrem() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        mdbLogWrite(LOG_WARNING, "mdbCommandLrem() key not exists | At %s:%d", __FILE__, __LINE__);
        // 不存在则回复客户端nil
        if(mdbSendReply(fd, "nil\r\n", MDB_REP_NIL) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrem() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLrem() list length: %d", l->len);
    // 获取要移除的数量
    long count = 0;
    if(mdbIsStringRepresentableAsLong(((SDS *)(c->argv[2]->ptr)), &count) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid count\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrem() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 如果count 小于 0， 从链表尾部开始移除，否则从链表头部开始移除
    int tmpCount = count;
    if(tmpCount < 0) {
        tmpCount = -tmpCount;
        listNode *node = mdbListLast(l);
        while(node != NULL && tmpCount > 0) {
            mobj *obj = node->value;
            listNode *tmpNode = mdbListPrevNode(node);
            mdbLogWrite(LOG_DEBUG, "mdbCommandLrem() obj: %s", obj == NULL ? "NULL": "NOT NULL");
            if(mdbCompareStringObjects(obj, c->argv[3]) == 0) {
                mdbListDelNode(l, node);
                tmpCount--;
            }
            node = tmpNode;
        }
    } else {
        listNode *node = mdbListFirst(l);
        while(node != NULL && tmpCount > 0) {
            mobj *obj = node->value;
            listNode *tmpNode = mdbListNextNode(node);
            mdbLogWrite(LOG_DEBUG, "mdbCommandLrem() obj: %s", obj == NULL ? "NULL": "NOT NULL");
            if(mdbCompareStringObjects(obj, c->argv[3]) == 0) {
                mdbListDelNode(l, node);
                tmpCount--;
            }
            node = tmpNode;
        }
    }
    // 发送移除的数量
    char buf[32];
    sprintf(buf, "%d\r\n", abs((int)count) - tmpCount);
    if(mdbSendReply(fd, buf, strlen(buf)) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLrem() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // AOF 追加
    mdbAppendAOF(c);
}
// LTRIM	修剪列表在指定的范围内。
// 例如：LTRIM key start stop
void mdbCommandLtrim(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 4) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'ltrim' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLtrim() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        if(mdbSendReply(fd, "ERR: no such key\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLtrim() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 从列表对象中获取列表
    linkedList *l = listObj->ptr;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLtrim() list length: %d", l->len);
    // 获取start
    long start = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &start) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid start\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLtrim() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取stop
    long stop = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[3]->ptr), &stop) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid stop\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLtrim() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "mdbCommandLtrim() start: %ld", start);
    mdbLogWrite(LOG_DEBUG, "mdbCommandLtrim() stop: %ld", stop);
    int len = mdbListLength(l);
    start = start < 0 ? len + start : start;
    stop = stop < 0 ? len + stop : stop;
    int tmp = max(start, stop);
    start = min(start, stop);
    stop = tmp;
    // 修剪列表
    l = mdbListTrim(l, start, stop);
    if(l == NULL) {
        // 修剪失败
        if(mdbSendReply(fd, "ERR: trim list failed\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLtrim() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 发送OK
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLtrim() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // AOF 追加
    mdbAppendAOF(c);
}
// LSET	设置列表中指定索引的元素。
// 例如：LSET key index value
void mdbCommandLset(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 4) {
        if(mdbSendReply(fd, "ERR wrong number of arguments for 'lset' command\r\n", MDB_REP_ERROR) < 0) {
            mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key 获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        if(mdbSendReply(fd, "(empty array)\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取列表
    linkedList *l = listObj->ptr;
    if(l->len == 0 || l == NULL || l->head == NULL) {
        if(mdbSendReply(fd, "(empty array)\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取 index
    long index = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &index) < 0) {
        if(mdbSendReply(fd, "(error) ERR value is not an integer or out of range\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "mdbCommandLset() | index: %ld", index);
    if(index < 0 || index >= l->len) {
        if(mdbSendReply(fd, "Out of range\r\n", MDB_REP_OK) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据index获取节点
    listNode *node = mdbListIndex(l, index);
    // 将节点的值替换
    mdbDecrRefCount((mobj *)node->value);
    mobj *newVal = mdbDupStringObject(c->argv[3]);
    node->value = newVal;
    // 发送回复
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLset() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    // AOF 追加
    mdbAppendAOF(c);
}
// LRANGE	返回指定范围的列表元素。
// 例如：LRANGE key start stop
void mdbCommandLrange(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 4) {
        if(mdbSendReply(fd, "ERR: wrong number of arguments for 'lrange' command\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 根据key获取列表对象
    mobj *listObj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(listObj == NULL) {
        if(mdbSendReply(fd, "(empty array)\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 检查是否是列表对象
    if(listObj->type != MDB_LIST) {
        // 不是列表对象
        if(mdbSendReply(fd, "ERR: operation against a key holding the wrong kind of value\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandRpush() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取链表
    linkedList *l = listObj->ptr;
    if(l->len <= 0 || l->head == NULL) {
        if(mdbSendReply(fd, "(empty array)\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    } 
    // 获取start
    long start = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[2]->ptr), &start) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid start\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    // 获取stop
    long stop = 0;
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[3]->ptr), &stop) < 0) {
        // 转换失败
        if(mdbSendReply(fd, "ERR: invalid stop\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    int len = mdbListLength(l);
    start = start < 0 ? len + start : start;
    stop = stop < 0 ? len + stop : stop;
    int tmp = max(start, stop);
    start = min(start, stop);
    stop = tmp;
    mdbLogWrite(LOG_DEBUG, "mdbCommandLrange() start: %ld", start);
    mdbLogWrite(LOG_DEBUG, "mdbCommandLrange() stop: %ld", stop);
    // 获取范围内的元素
    listNode *node = mdbListIndex(l, start);
    SDS *retArray = mdbSdsNewempty();
    while(node != NULL && start <= stop) {
        mobj *obj = node->value;
        mdbLogWrite(LOG_DEBUG, "mdbCommandLrange() obj: %s", obj == NULL ? "NULL": "NOT NULL");
        retArray = mdbSdscat(retArray, ((SDS *)(obj->ptr))->buf);
        retArray = mdbSdscat(retArray, "\r\n");
        mdbLogWrite(LOG_DEBUG, "-------1");
        node = mdbListNextNode(node);
        start++;
    }
    mdbLogWrite(LOG_DEBUG, "-------2");
    if(mdbSdslen(retArray) <= 0) {
        if(mdbSendReply(fd, "(empty array)\r\n", MDB_REP_ERROR) < 0) {
            // 发送失败
            mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        }
        return;
    }
    mdbLogWrite(LOG_DEBUG, "-------3");
    // 发送数组
    if(mdbSendReply(fd, retArray->buf, MDB_REP_ARRAY) < 0) {
        // 发送失败
        mdbLogWrite(LOG_ERROR, "mdbCommandLrange() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
    }
    mdbSdsfree(retArray);
}