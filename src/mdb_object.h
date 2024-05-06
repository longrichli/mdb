#ifndef __MDB_OBJECT_H__
#define __MDB_OBJECT_H__

#include <stdlib.h>

#define MDB_SHARED_INTEGERS (10000)

typedef enum mdbObjType {
    MDB_STRING = 0,
    MDB_LIST = 1,
    MDB_HASH = 2,
    MDB_SET = 3,
    MDB_ZSET = 4
} mobjType;

typedef enum mdbObjEncding {
    MDB_ENCODING_RAW = 0,
    MDB_ENCODING_INT = 1,
    MDB_ENCODING_EMBSTR = 2,
    MDB_ENCODING_HT = 3,
    MDB_ENCODING_LINKEDLIST = 4,
    MDB_ENCODING_ZIPLIST = 5,
    MDB_ENCODING_INTSET = 6,
    MDB_ENCODING_SKIPLIST = 7
} mobjEncoding;

typedef struct mdbObject {
    mobjType type;
    mobjEncoding encoding;
    int refCount;
    unsigned int lru;
    void *ptr;
}mobj;

typedef struct sharedObject{
    mobj *integers[MDB_SHARED_INTEGERS];
} sharedObject;

/*
des:
    对象引用 + 1
param:
    obj: 目标对象
*/
void mdbIncrRefCount(mobj *obj);

/*
des:
    对象引用 -1, 如果降到0, 说明没有引用此对象的指针, 将释放内存
param:
    obj: 目标对象
*/
void mdbDecrRefCount(mobj *obj);

/*
des: 
    释放字符串对象
param:
    obj: 目标对象
*/
void mdbFreeStringObj(mobj *obj);

/*
des:
    释放链表对象
param:
    obj: 目标对象
*/
void mdbFreeListObject(mobj *obj);

/*
des:
    释放集合对象
param:
    obj: 目标对象
*/
void mdbFreeSetObject(mobj *obj);

/*
des:
    释放有序集合对象
param:
    obj: 目标对象
*/
void mdbFreeZsetObject(mobj *obj);

/*
des:
    释放哈希对象
param:
    obj: 目标对象
*/
void mdbFreeHashObject(mobj *obj);

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
mobj *mdbCreateObject(mobjType type, void *ptr);

/*
des:
    创建字符串对象
param:
    ptr: 字符串内容
return:
    成功: 指定类型的对象
    失败: NULL
*/
mobj *mdbCreateStringObject(char *ptr);

/*
des:
    复制字符串对象
param:
    obj: 待复制的对象
return:
    成功: 新的字符串对象
    失败: NULL
*/
mobj *mdbDupStringObject(mobj *obj);

/*
des: 
    尝试对字符串对象进行编码, 来节省内存
param:
    obj: 字符串对象
return:
    成功: 编码后的字符串对象
    失败: NULL
*/
mobj *mdbTryObjectEncoding(mobj *obj);

/*
des: 
    获取一个对象的解码版本 , 如果对象已经是解码版本,就引用计数加1
param:
    obj: 字符串对象
return:
    成功: 编码后的字符串对象
    失败: NULL
*/
mobj *mdbGetDecodedObject(mobj *obj);

/*
des:
    获取字符串对象的长度
param:
    obj: 字符串对象
return:
    字符串长度
*/
size_t mdbStringObjectLen(mobj *obj);

/*
des:
    从longlong型创建字符串对象
param:
    value: long long 型数字
return:
    字符串对象
*/
mobj *mdbCreateStringObjectFromLongLong(long long value);

/*
des:
    创建列表对象
return:
    成功: 列表对象
    失败: NULL
*/
mobj *mdbCreateListObject(void);

/*
des:
    创建压缩列表对象
return:
    成功: 压缩列表对象
    失败: NULL
*/
mobj *mdbCreateZiplistObject(void);

/*
des:
    创建集合对象
return:
    成功: 集合对象
    失败: NULL
*/
mobj *mdbCreateSetObject(void);

/*
des:
    创建整数集合对象
return:
    成功: 整数集合对象
    失败: NULL
*/
mobj *mdbCreateIntsetObject(void);

/*
des:
    创建哈希对象
return:
    成功: 哈希对象
    失败: NULL
*/
mobj *mdbCreateHashObject(void);

/*
des:
    创建有序集合对象
return:
    成功: 有序集合对象
    失败: NULL
*/
mobj *mdbCreateZsetObject(void);

// int getLongFromObjectOrReply(redisClient *c, mobj *o, long *target, const char *msg);
// int checkType(redisClient *c, mobj *o, int type);
// int getLongLongFromObjectOrReply(redisClient *c, mobj *o, long long *target, const char *msg);
// int getDoubleFromObjectOrReply(redisClient *c, mobj *o, double *target, const char *msg);

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
int mdbGetLongLongFromObject(mobj *obj, long long *target);

/*
des:
    获取编码的字符串表示
param:
    encoding: 编码
return:
    编码的字符串表示
*/
char *mdbStrEncoding(mobjEncoding encoding);

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
int mdbCompareStringObjects(mobj *a, mobj *b);

/*
des:
    给定一个对象，使用近似的LRU算法返回对象从未被请求的最小秒数。
param:
    obj: 给定对象
return:
    从未被请求的最小秒数
*/
unsigned long mdbEstimateObjectIdleTime(mobj *obj);


/*
des:
    创建共享对象
return:
    成功: 0
    失败: -1
*/
int mdbCreateSharedObject(void);

/*
des:
    比较对象
param:
    v1: 对象1
    v2: 对象2
return:
    v1 == v2: 0
    v1 < v2 : < 0
    v1 > v2 : > 0
*/
int mdbDictMdbObjCompare(const void *v1, const void *v2);

/*
des:
    比较SDS key
param:
    key1: key1
    key2: key2
return:
    key1 == key2: 0
    key1 < key2 : < 0
    key1 > key2 : > 0
*/
int mdbDictSdsKeyCompare(const void *key1, const void *key2);

/*
des:
    计算对象的hash值
param:
    v: 对象
return:
    hash值

*/
unsigned int mdbHashFun(const void *v);
/*
des:
    释放对象
param:
    obj: 对象
*/
void mdbDictMdbObjFree(void *obj);





#endif /* __MDB_OBJECT_H__ */