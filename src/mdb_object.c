#include "mdb_common.h"
#include "mdb_alloc.h"
#include "mdb_sds.h"
#include "mdb_list.h"
#include "mdb_dict.h"
#include "mdb_intset.h"
#include "mdb_object.h"
#include "mdb_util.h"
#include "mdb_skiplist.h"
#include <limits.h>
#include <time.h>

sharedObject gshared;

unsigned int mdbHashFun(const void *v) {
    mobj *key = (mobj *)v;
    if(key == NULL || key->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbSetHashFun() | At %s:%d", __FILE__, __LINE__);
        return 0;
    }
    if(key->encoding == MDB_ENCODING_EMBSTR || key->encoding == MDB_ENCODING_RAW) {
        return mdbBkdrHash(((SDS *)key->ptr)->buf);
    } else if(key->encoding == MDB_ENCODING_INT) {
        char buf[32] = {0};
        mdbLonglong2Stirng(buf, 32, (long long)key->ptr);
        return mdbBkdrHash(buf);
    } else {
        mdbLogWrite(LOG_ERROR, "mdbSetHashFun() | At %s:%d", __FILE__, __LINE__);
        return 0;
    }
}

void mdbDictMdbObjFree(void *obj) {
    if(obj != NULL) {
        mdbDecrRefCount(obj);
    }
}

/*
des:
    key为SDS时的比较函数
param:
    key1: key1
    key2: key2
return:
    key1 = key2: 0
    key1 != key2: !0
*/
int mdbSdsKeyCompare(const void *key1, const void *key2) {
    if(key1 == NULL && key2 == NULL) {
        return 0;
    } else if(key1 == NULL || key2 == NULL) {
        return 1;
    }
    int l1,l2;
    l1 = mdbSdslen((SDS *)key1);
    l2 = mdbSdslen((SDS *)key2);
    if (l1 != l2) return 1;
    return memcmp(((SDS *)key1)->buf, ((SDS *)key2)->buf, l1);
}

/*
des:
    key为字符串对象时的比较函数
param:
    key1: key1
    key2: key2
return:
    key1 = key2: 0
    key1 != key2: !0
*/
int mdbStringObjKeyCompare(const void* key1, const void *key2) {
    int cmp = 0;
    mobj *obj1 = (mobj *)key1;
    mobj *obj2 = (mobj *)key2;
    if(obj1 == NULL && obj2 == NULL) {
        return 0;
    } else if(obj1 == NULL || obj2 == NULL) {
        return 1;
    }
    if(obj1->encoding == MDB_ENCODING_RAW || obj2->encoding == MDB_ENCODING_RAW) {
        mdbLogWrite(LOG_DEBUG, "obj1->ptr: %s, obj2->ptr: %s", obj1->ptr == NULL ? "NULL" : "NOT NULL", obj2->ptr == NULL ? "NULL" : "NOT NULL");
        return mdbSdsKeyCompare(obj1->ptr, obj2->ptr);
    } else {
        mobj *o1 = mdbGetDecodedObject(obj1);
        mobj *o2 = mdbGetDecodedObject(obj2);
        cmp = mdbSdsKeyCompare(o1->ptr, o2->ptr);
        mdbDecrRefCount(o1);
        mdbDecrRefCount(o2);
    }
    return cmp;
}

dictType gSetDtype = {
    mdbHashFun,           // hash function
    NULL,                 // keydup
    NULL,                 // valdup
    mdbStringObjKeyCompare, // keyCompare
    mdbDictMdbObjFree,    // keyFree
    mdbDictMdbObjFree     // valFree
};

dictType gHashDtype = {
    mdbHashFun,           // hash function
    NULL,                 // keydup
    NULL,                 // valdup
    mdbStringObjKeyCompare, // keyCompare
    mdbDictMdbObjFree,    // keyFree
    mdbDictMdbObjFree     // valFree
};

/*
des:
    对象引用 + 1
param:
    obj: 目标对象
*/
void mdbIncrRefCount(mobj *obj) {
    if(obj == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbIncrRefCount() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    obj->refCount++;
}

/*
des:
    对象引用 -1, 如果降到0, 说明没有引用此对象的指针, 将释放内存
param:
    obj: 目标对象
*/
void mdbDecrRefCount(mobj *obj) {
    if(obj == NULL || obj->refCount == 0) {
        mdbLogWrite(LOG_ERROR, "mdbDecrRefCount() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    obj->refCount--;
    if(obj->refCount <= 0) {
        // 释放对象
        mobjType type = obj->type;
        switch (type) {
            case MDB_STRING:
                mdbFreeStringObj(obj);
                break;
            case MDB_LIST:
                mdbFreeListObject(obj);
                break;
            case MDB_HASH:
                mdbFreeHashObject(obj);
                break;
            case MDB_SET:
                mdbFreeSetObject(obj);
                break;
            case MDB_ZSET:
                mdbFreeZsetObject(obj);
                break;
            default:
                mdbLogWrite(LOG_ERROR, "Error: Unknown Type | At %s:%d", __FILE__, __LINE__);
                break;
        }
        obj->ptr = NULL;
        mdbFree(obj);
    }
}

/*
des: 
    释放字符串对象
param:
    obj: 目标对象
*/
void mdbFreeStringObj(mobj *obj) {
    if(obj == NULL || obj->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbFreeStringObj() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    // if(obj->encoding == MDB_ENCODING_INT) 存储的是整数, 没有开辟内存,无需释放
    if(obj->encoding == MDB_ENCODING_RAW) {
        //释放SDS内存
        mdbSdsfree((SDS *)obj->ptr);
    }
    // if(obj->encoding == MDB_ENCODING_EMBSTR) obj 和 SDS绑定在一起, 在外面释放
}

/*
des:
    释放链表对象
param:
    obj: 目标对象
*/
void mdbFreeListObject(mobj *obj) {
    if(obj == NULL || obj->type != MDB_LIST) {
        mdbLogWrite(LOG_ERROR, "mdbFreeListObject() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    if(obj->encoding == MDB_ENCODING_ZIPLIST) {
        // 目前不支持压缩列表
        return;
    } else if(obj->encoding == MDB_ENCODING_LINKEDLIST) {
        mdbListFree((linkedList *)obj->ptr);
    }

}

/*
des:
    释放集合对象
param:
    obj: 目标对象
*/
void mdbFreeSetObject(mobj *obj) {
    if(obj == NULL || obj->type != MDB_SET) {
        mdbLogWrite(LOG_ERROR, "mdbFreeSetObject() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    if(obj->encoding == MDB_ENCODING_INTSET) {
        mdbIntsetFree((intset *)obj->ptr);
    } else if(obj->encoding == MDB_ENCODING_HT) {
        mdbDictFree((dict *)obj->ptr);
    }
}

/*
des:
    释放有序集合对象
param:
    obj: 目标对象
*/
void mdbFreeZsetObject(mobj *obj) {
    if(obj == NULL || obj->type != MDB_ZSET) {
        mdbLogWrite(LOG_ERROR, "mdbFreeZsetObject() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    // zset功能
    if(obj->encoding == MDB_ENCODING_ZIPLIST) {
        // 释放zlplist内存
    } else if(obj->encoding == MDB_ENCODING_SKIPLIST) {
        // 释放skiplist内存
        mdbSkipListFree(((zset *)obj->ptr)->sl);
        // 释放dict内存
        mdbDictFree(((zset *)obj->ptr)->d);
        // 释放zset内存
        mdbFree(obj->ptr); 
    }
}

/*
des:
    释放哈希对象
param:
    obj: 目标对象
*/
void mdbFreeHashObject(mobj *obj) {
    if(obj == NULL || obj->type != MDB_HASH) {
        mdbLogWrite(LOG_ERROR, "mdbFreeHashObject() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    if(obj->encoding == MDB_ENCODING_ZIPLIST) {
        // 释放ziplist内存
    } else if(obj->encoding == MDB_ENCODING_HT) {
        mdbDictFree((dict *)obj->ptr);
    }
}

/*
des:
    创建指对象
param:
    type: 对象类型
    ptr: 指向对象内容
return:
    成功: 指定类型的对象
    失败: NULL
*/
mobj *mdbCreateObject(mobjType type, void *ptr) {
    int ret = -1;
    mobj *obj = NULL;
    // if(ptr == NULL) {
    //     mdbLogWrite(LOG_ERROR, "mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
    //     goto __finish;
    // }
    obj = mdbMalloc(sizeof(*obj));
    if(obj == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateObject() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj->type = type;
    obj->ptr = ptr;
    obj->refCount = 1;
    obj->lru = time(NULL);
    obj->encoding = MDB_ENCODING_RAW;
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

/*
des:
    创建字符串对象
param:
    ptr: 字符串内容
return:
    成功: 指定类型的对象
    失败: NULL
*/
mobj *mdbCreateStringObject(char *ptr) {
    return mdbCreateObject(MDB_STRING, mdbSdsnew(ptr));
}

/*
des:
    创建 EMPSTR 字符串对象
param: 
    ptr: 字符串内容
return: 
    成功: 指定类型的对象
    失败: NULL
*/
mobj *mdbCreateEmpstrObject(char *ptr) {
    int ret = -1;
    mobj *obj = NULL;
    SDS *sds = NULL;
    if(ptr == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEmpstrObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 将mobj 和 sds 创建在一起
    obj = mdbMalloc(sizeof(obj) + sizeof(SDS) + strlen(ptr) + 1); 
    if(obj == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEmpstrObject() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj->type = MDB_STRING;
    obj->encoding = MDB_ENCODING_EMBSTR;
    obj->refCount = 1;
    obj->lru = time(NULL);
    sds = (SDS *)(obj + sizeof(obj));
    sds->len = strlen(ptr);
    sds->free = 0;
    strcpy(sds->buf, ptr);
    obj->ptr = sds;
    
    ret = 0;
    
__finish:
    return ret == 0 ? obj : NULL;
}

/*
des:
    复制字符串对象
param:
    obj: 待复制的对象
return:
    成功: 新的字符串对象
    失败: NULL
*/
mobj *mdbDupStringObject(mobj *obj) {
    mobj *newObj = NULL;
    if(obj == NULL || obj->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbDupStringObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(obj->encoding != MDB_ENCODING_RAW) {
        mdbLogWrite(LOG_ERROR, "mdbDupStringObject() error: Unsupport Other Encoding | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    newObj = mdbCreateStringObject(((SDS *)obj->ptr)->buf);
    if(newObj == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDupStringObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
__finish:
    return newObj;
}

/*
des: 
    尝试对字符串对象的RAW编码转换为 INT编码, 或使用共享对象 来节省内存
param:
    obj: 字符串对象
return:
    成功: 编码后的字符串对象
    失败: NULL
*/
mobj *mdbTryObjectEncoding(mobj *obj) {
    long lval = 0;
    if(obj == NULL || obj->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbTryObjectEncoding() | At %s:%d", __FILE__, __LINE__);
        return obj;
    }
    if(obj->encoding != MDB_ENCODING_RAW || obj->refCount > 1) {
        return obj;
    }
    // 如何可以转换成long类型, 就获取long类型值
    if(mdbIsStringRepresentableAsLong((SDS *)obj->ptr, &lval) < 0) {
        return obj;
    }
    // 已经转换成long类型, 如果lval 的值在 0到 MDB_SHARED_INTEGERS 之间, 使用共享字符串对象
    if(lval >= 0 && lval < MDB_SHARED_INTEGERS) {
        // 使用共享对象, 明天写
        mdbDecrRefCount(obj);
        obj = gshared.integers[lval];
        mdbIncrRefCount(gshared.integers[lval]);
    } else {
        obj->encoding = MDB_ENCODING_INT;
        mdbSdsfree(obj->ptr);
        obj->ptr = (void *)lval;
    }
    return obj;
}

/*
des: 
    获取一个字符串对象的解码(RAW)版本 , 如果对象已经是解码版本,就引用计数加1
param:
    obj: 字符串对象
return:
    成功: 编码后的字符串对象
    失败: NULL
*/
mobj *mdbGetDecodedObject(mobj *obj) {
    int ret = -1;
    mobj *dec = NULL;
    char buf[32] = {0};
    if(obj == NULL || obj->type != MDB_STRING) {
        mdbLogWrite(LOG_DEBUG, obj == NULL ? "obj is NULL" : "obj is not NULL");
        if(obj != NULL)
        mdbLogWrite(LOG_DEBUG, obj->type == MDB_STRING ? "obj is string" : "obj is not string");
        mdbLogWrite(LOG_ERROR, "mdbGetDecodedObject() | At %s:%d", __FILE__, __LINE__);
        mdbLogWrite(LOG_DEBUG, "obj->encoding : %d", obj->encoding);
        mdbLogWrite(LOG_DEBUG, "obj->type : %d", obj->type);
        mdbLogWrite(LOG_DEBUG, "obj->ptr : %s", obj->ptr == NULL ? "NULL" : "NOT NULL");
        mdbLogWrite(LOG_DEBUG, "obj->refCount : %d", obj->refCount);
        goto __finish;
    }
    if(obj->encoding == MDB_ENCODING_RAW) {
        mdbIncrRefCount(obj);
        dec = obj;
        ret = 0;
        goto __finish;
    } else if(obj->encoding == MDB_ENCODING_INT) {
        mdbLonglong2Stirng(buf, 32, (long long)obj->ptr);
        dec = mdbCreateStringObject(buf);
        if(dec == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbGetDecodedObject() mdbCreateStringObject() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
    } else if(obj->encoding == MDB_ENCODING_EMBSTR) {
        dec = mdbCreateStringObject(((SDS *)obj->ptr)->buf);
        ret = 0;
        goto __finish;
    } else {
        mdbLogWrite(LOG_ERROR, "mdbGetDecodedObject(): Unkonwn Encoding | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret == 0 ? dec : NULL;
}

/*
des:
    获取字符串对象的长度
param:
    obj: 字符串对象
return:
    字符串长度
*/
size_t mdbStringObjectLen(mobj *obj) {
    if(obj == NULL || obj->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbStringObjectLen() | At %s:%d", __FILE__, __LINE__);
        return 0;
    }
    if(obj->encoding == MDB_ENCODING_RAW || obj->encoding == MDB_ENCODING_EMBSTR) {
        SDS *sds = (SDS *)obj->ptr;
        return sds->len;
    } else if(obj->encoding == MDB_ENCODING_INT) {
        char buf[32] = {0};
        return mdbLonglong2Stirng(buf, 32, (long long)obj->ptr);
    } else {
        mdbLogWrite(LOG_ERROR, "mdbStringObjectLen(): Unkonwn Encoding | At %s:%d", __FILE__, __LINE__);
        return 0;
    }
}

/*
des:
    从longlong型创建字符串对象
param:
    value: long long 型数字
return:
    字符串对象
*/
mobj *mdbCreateStringObjectFromLongLong(long long value) {
    mobj *obj = NULL;
    int ret = -1;
    if(value >= 0 && value < MDB_SHARED_INTEGERS) {
        mdbLogWrite(LOG_DEBUG, "mdbCreateStringObjectFromLongLong() | Use Shared Object");
        // 使用共享对象
        obj = gshared.integers[value];
        mdbLogWrite(LOG_DEBUG, "obj : %s", obj == NULL ? "NULL" : "NOT NULL");
    } else if(value >= LONG_MIN && value <= LONG_MAX) {
        obj = mdbCreateObject(MDB_STRING, (void *)value);
        if(obj == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbCreateStringObjectFromLongLong() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
        obj->encoding = MDB_ENCODING_INT;
    } else {
        char buf[32] = {0};
        mdbLonglong2Stirng(buf, 32, value);
        obj = mdbCreateStringObject(buf);
        if(obj == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbCreateStringObjectFromLongLong() mdbCreateStringObject() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
    }
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

void mdbFreeListObjElement(void *ptr) {
    if(ptr != NULL) {
        mdbDecrRefCount(ptr);
    }
}

int mdbMatchListObjElement(void *a, void *b) {
    return mdbStringObjKeyCompare(a, b);
}

/*
des:
    创建列表对象
return:
    成功: 列表对象
    失败: NULL
*/
mobj *mdbCreateListObject(void) {
    linkedList *list = NULL;
    int ret = -1;
    mobj *obj = NULL;
    list = mdbListCreate(NULL, mdbFreeListObjElement, mdbMatchListObjElement);
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateListObject() mdbListCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj = mdbCreateObject(MDB_LIST, list);
    if(obj == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateListObject() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
        mdbListFree(list);
        goto __finish;
    }
    obj->encoding = MDB_ENCODING_LINKEDLIST;
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

/*
des:
    创建压缩列表对象
return:
    成功: 压缩列表对象
    失败: NULL
*/
mobj *mdbCreateZiplistObject(void) {
    // 压缩列表还未实现, 先返回NULL
    return NULL;
}

/*
des:
    创建集合对象
return:
    成功: 集合对象
    失败: NULL
*/
mobj *mdbCreateSetObject(void) {
    dict *d = NULL;
    int ret = -1;
    mobj *obj = NULL;
    d = mdbDictCreate(&gSetDtype);
    if(d == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateSetObject() mdbDictCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj = mdbCreateObject(MDB_SET, d);
    if(obj == NULL) {
        mdbDictFree(d);
        mdbLogWrite(LOG_ERROR, "mdbCreateSetObject() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj->encoding = MDB_ENCODING_HT;
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

/*
des:
    创建整数集合对象
return:
    成功: 整数集合对象
    失败: NULL
*/
mobj *mdbCreateIntsetObject(void) {
    int ret = -1;
    intset *iset = NULL;
    mobj *obj = NULL;
    iset = mdbIntsetNew();
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateIntsetObject() mdbIntsetNew() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj = mdbCreateObject(MDB_SET, iset);
    if(obj == NULL) {
        mdbIntsetFree(iset);
        mdbLogWrite(LOG_ERROR, "mdbCreateIntsetObject() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj->encoding = MDB_ENCODING_INTSET;
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

/*
des:
    创建哈希对象
return:
    成功: 哈希对象
    失败: NULL
*/
mobj *mdbCreateHashObject(void) {
    dict *d = NULL;
    int ret = -1;
    mobj *obj = NULL;
    d = mdbDictCreate(&gHashDtype);
    if(d == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateSetObject() mdbDictCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj = mdbCreateObject(MDB_HASH, d);
    if(obj == NULL) {
        mdbDictFree(d);
        mdbLogWrite(LOG_ERROR, "mdbCreateSetObject() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj->encoding = MDB_ENCODING_HT;
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

dictType gZsetDictType = {
    mdbHashFun,            // hash
    NULL,                 // keydup
    NULL,                 // valdup
    mdbStringObjKeyCompare, // keyCompare
    mdbDictMdbObjFree,    // keyFree
    mdbDictMdbObjFree     // valFree
};
/*
des:
    创建有序集合对象
return:
    成功: 有序集合对象
    失败: NULL
*/
mobj *mdbCreateZsetObject(void) {
    int ret = -1;
    mobj *obj = NULL;
    zset *zst = NULL;
    zst = mdbMalloc(sizeof(zset));
    if(zst == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateZsetObject() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }

    zst->sl = mdbSkipListCreate();
    if(zst->sl == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateZsetObject() mdbSkipListCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    zst->d = mdbDictCreate(&gZsetDictType);
    if(zst->d == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateZsetObject() mdbDictCreate() | At %s:%d", __FILE__, __LINE__);
        mdbSkipListFree(zst->sl);
        goto __finish;
    }
    obj = mdbCreateObject(MDB_ZSET, zst);
    if(obj == NULL) {
        mdbSkipListFree(zst->sl);
        mdbDictFree(zst->d);
        mdbLogWrite(LOG_ERROR, "mdbCreateZsetObject() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    obj->encoding = MDB_ENCODING_SKIPLIST;
    ret = 0;
__finish:
    return ret == 0 ? obj : NULL;
}

/*
des:
    从字符串对象中获取long long 型整数
param:
    obj: 字符串对象
    target: 用来存放long long 型整数
return:
    成功: 0
    失败: -1
*/
int mdbGetLongLongFromObject(mobj *obj, long long *target) {
    int ret = -1;
    if(obj == NULL || obj->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbGetLongLongFromObject() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(mdbIsObjectRepresentableAsLongLong(obj, target) < 0) {
        mdbLogWrite(LOG_ERROR, "mdbGetLongLongFromObject() mdbIsObjectRepresentableAsLongLong() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

/*
des:
    获取对象类型
param:
    obj: 对象
return:
    对象类型
*/
char* mdbObjectType(mobj *obj) {
    switch(obj->type) {
        case MDB_STRING:
            return "string";
        case MDB_LIST:
            return "list";
        case MDB_SET:
            return "set";
        case MDB_HASH:
            return "hash";
        case MDB_ZSET:
            return "zset";
        default:
            return "unknown";
    }
}

/*
des:
    获取编码的字符串表示
param:
    encoding: 编码
return:
    编码的字符串表示
*/
char *mdbStrEncoding(mobjEncoding encoding) {
    switch(encoding) {
        case MDB_ENCODING_INT:
            return "int";
        case MDB_ENCODING_RAW:
            return "raw";
        case MDB_ENCODING_EMBSTR:
            return "empstr";
        case MDB_ENCODING_HT:
            return "hashtable";
        case MDB_ENCODING_INTSET:
            return "intset";
        case MDB_ENCODING_LINKEDLIST:
            return "linkedlist";
        case MDB_ENCODING_SKIPLIST:
            return "skiplist";
        case MDB_ENCODING_ZIPLIST:
            return "ziplist";
        default:
            mdbLogWrite(LOG_DEBUG, "Not fount encoding. | At %s:%d", __FILE__, __LINE__);
    }
    return "unknown";
}

/*
des:
    比较字符串对象
param:
    a: 字符串对象 a
    b: 字符串对象 b
return:
    a == b: 0
    a < b : < 0
    a > b : > 0
*/
int mdbCompareStringObjects(mobj *a, mobj *b) {
    int ret = -1;
    SDS *sdsa = NULL;
    SDS *sdsb = NULL;
    if(a == NULL || b == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCompareStringObjects() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(a->type != MDB_STRING || b->type != MDB_STRING) {
        mdbLogWrite(LOG_ERROR, "mdbCompareStringObjects() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(a->encoding == MDB_ENCODING_INT) {
        char buf[32] = {0};
        mdbLonglong2Stirng(buf, 32, (long long)a->ptr);
        sdsa = mdbSdsnew(buf);
        if(sdsa == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbCompareStringObjects() mdbSdsnew() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
    } 
    if(b->encoding == MDB_ENCODING_INT) {
        char buf[32] = {0};
        mdbLonglong2Stirng(buf, 32, (long long)b->ptr);
        sdsb = mdbSdsnew(buf);
        if(sdsb == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbCompareStringObjects() mdbSdsnew() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
    }
    ret = mdbSdsCmp(sdsa == NULL ? a->ptr : sdsa, sdsb == NULL ? b->ptr : sdsb);
__finish:
    mdbSdsfree(sdsa);
    mdbSdsfree(sdsb);
    return ret;
}

/*
des:
    给定一个对象，使用近似的LRU算法返回对象从未被请求的最小秒数。
param:
    obj: 给定对象
return:
    从未被请求的最小秒数
*/
unsigned long mdbEstimateObjectIdleTime(mobj *obj) {
    // 以后再说
    return 0;
}

/*
des:
    创建共享对象
return:
    成功: 0
    失败: -1
*/
int mdbCreateSharedObjects(void) {
    int ret = -1;
    for(long i = 0; i < MDB_SHARED_INTEGERS; i++) {
        gshared.integers[i] = NULL;
        gshared.integers[i] = mdbCreateObject(MDB_STRING, (void *)i);
        if(gshared.integers[i] == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbCreateSharedObjects() mdbCreateObject() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
        gshared.integers[i]->encoding = MDB_ENCODING_INT;
    }
    ret = 0;
__finish:
    return ret;
}