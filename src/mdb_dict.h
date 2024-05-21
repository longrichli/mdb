#ifndef __MDB_DICT_H__
#define __MDB_DICT_H__

#include <stdint.h>
#include <stdlib.h>
typedef struct dictEntry {
    void *key;                  /* key */
    void *val;                  /* void * 型的值 */
    struct dictEntry *next;     /* 指向下一个dictEntry 结构, 形成链表 */
} dictEntry;

typedef struct dictht {
    dictEntry **table;  /* hash 表数组 */
    int sz;          /* hash 表大小 */
    int mask;        /* hash 表大小掩码, 总是等于 sz -1 */
    int used;        /* 该hash 表已有节点的数量 */
} dictht;

typedef struct dictType {
    unsigned int (*hashFunction)(const void *key);          /* 计算 hash 值的函数 */
    void *(*keyDup)(const void *key);                       /* 复制 key 的函数 */
    void *(*valDup)(const void *val);                       /* 复制 val 的函数 */
    int (*keyCompare)(const void *key1, const void *key2);  /* 对比 key 的函数 */ 
    void (*keyFree)(void *key);                       /* 释放 key 的函数 */
    void (*valFree)(void *val);                       /* 释放 val 的函数*/
} dictType;

typedef struct dict {
    dictType *type;                                         /* 类型特定函数 */
    dictht ht[2];                                           /* hash 表*/
    int trehashidx;                                         /* rehash 索引, 当 rehash 不再进行时, 值为 -1 */
} dict;

/*
des:
    BKDR哈希函数
param:
    key: key
return:
    hash 值
*/
unsigned int mdbBkdrHash(const void *key);
/*
des:
    创建一个字典
param:
    type: 对字典中key 和 value 的相关操作函数 (keyfree , value free 等)
return:
    成功: 新的字典地址
    失败: NULL
*/
dict *mdbDictCreate(dictType * type);


/*
des:
    添加一个键值对, 如果已经存在键,返回错误
param:
    d: 字典
    key: key
    val: value
return:
    成功: 0
    失败: -1
*/
int mdbDictAdd(dict *d, void *key, void *val);

/*
des:
    添加一个键值对, 如果已经存在键,则将值替换成新的值
param:
    d: 字典
    key: key
    val: value
return:
    成功: 0
    失败: -1
*/
int mdbDictReplace(dict *d, void *key, void *val);

/*
des:
    拿到key对应的值
param:
    d: 字典
    key: key
return:
    成功: key对应的值
    失败: NULL
*/
void *mdbDictFetchValue(dict *d, void *key);

/*
des:
    删除key对应的键值对
param:
    d: 字典
    key: key
return:
    成功: 0
    失败: -1
*/
int mdbDictDelete(dict *d, void *key);

/*
des:
    释放字典
param:
    d: 字典
*/
void mdbDictFree(dict *d);



#endif /* __MDB_DICT_H__ */