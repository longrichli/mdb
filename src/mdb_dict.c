#include "mdb_common.h"
#include "mdb_dict.h"
#include "mdb_alloc.h"
#define DICT_INIT_SIZE (4)
/* 扩展负载因子 */
#define EXPENT_LOAD_FACTOR (1.0)
/* 缩小负载因子 */
#define SHRINK_LOAD_FACTOR (0.1)
static void initDictht(dictht *ht) {
    ht->mask = 0;
    ht->sz = 0;
    ht->table = NULL;
    ht->used = 0;
}
/*
des:
    创建一个字典
param:
    type: 对字典中key 和 value 的相关操作函数 (keyfree , value free 等)
return:
    成功: 新的字典地址
    失败: NULL
*/
dict *mdbDictCreate(dictType *type) {
    int ret = -1;
    dict *d = NULL;
    if(type == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    d = mdbMalloc(sizeof(*d));
    if(d == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictCreate() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    initDictht(&(d->ht[0]));
    initDictht(&(d->ht[1]));
    d->type = type;
    d->trehashidx = -1;
    ret = 0;
__finish:
    return ret == 0 ? d : NULL;
}

int mdbDictResize(dict *d) {
    int ret = -1;
    size_t realSize = DICT_INIT_SIZE;
    float loadFactor = 0.0;
    int sz = 0;
    if(d == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictResize() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    loadFactor = (float)d->ht[0].used/d->ht[0].sz;
    if(loadFactor >= EXPENT_LOAD_FACTOR) {
        /* 扩展 */
        sz = d->ht[0].sz * 2;
    } else if(loadFactor <= SHRINK_LOAD_FACTOR) {
        /* 缩小 */
        sz = d->ht[0].sz / 2;
    } else {
        /* 不用管 */
        ret = 0;
        goto __finish;
    }
    while(realSize < sz) {
        /* 真实大小 */
        realSize << 1; 
    }
    /* 如果比 DICT_INIT_SIZE 还小, 就为 DICT_INIT_SIZE */
    realSize = realSize < DICT_INIT_SIZE ? DICT_INIT_SIZE : realSize;
    d->ht[1].table = mdbMalloc(realSize * sizeof(dictEntry *));
    memset(d->ht[1].table, 0, realSize * sizeof(dictEntry *));
    if(mdbMalloc == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictResize() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    d->ht[1].sz = realSize;
    d->ht[1].used = 0;
    d->ht[1].mask = realSize - 1;
    // 惰性进行 rehash
    d->trehashidx = 0;
    ret = 0;
__finish:
    return ret;
}

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
int mdbDictAdd(dict *d, void *key, void *val) {
    int ret = -1;
    int htIdx = 0;
    size_t tableIdx = 0;
    unsigned int hash = 0;
    dictEntry *newEntry = NULL;
    dictEntry *tmpEntry = NULL;
    if(d == NULL || key == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictAdd() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    mdbDictResize(d);
    if(d->trehashidx >= d->ht[0].sz) {
        /* rehash 结束, 将ht[1] 的内容放到 ht[0] 上, 然后重置 ht[1], trehashidx 改为 -1 */
        d->ht[0].sz = d->ht[1].sz;
        d->ht[0].used = d->ht[1].used;
        d->ht[0].mask = d->ht[1].mask;
        d->ht[0].table = d->ht[1].table;
        initDictht(&(d->ht[1]));
        d->trehashidx = -1;
    } else if(d->trehashidx >= 0) {
        /* 进行 rehash, 将ht[0].table[trehashidx] 指向的链表中的值都插入到 ht[1]中 */
        dictEntry *entry = d->ht[0].table[d->trehashidx];
        while(entry != NULL) {
            /* 临时保存一下 */
            tmpEntry = entry->next; 
            hash = d->type->hashFunction(entry->key);
            tableIdx = hash & d->ht[1].mask;
            entry->next = d->ht[1].table[tableIdx];
            d->ht[1].table[tableIdx] = entry;
            d->ht[1].used++;
            entry = tmpEntry;
        }
        
    }
    /* 选择往哪个 hash 表中添加, 如果 trehashidx 不等于 -1 , 说明正在rehash, 就向 ht[1] 添加, 否则就 ht[0] */
    htIdx = d->trehashidx == -1 ? 0 : 1;
    newEntry = mdbMalloc(sizeof(*newEntry));
    if(newEntry == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictAdd() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    newEntry->key = key;
    newEntry->val = val;
    hash = d->type->hashFunction(newEntry->key);
    tableIdx = hash & d->ht[htIdx].mask;
    newEntry->next = d->ht[htIdx].table[tableIdx];
    d->ht[htIdx].table[tableIdx] = newEntry;
    d->ht[htIdx].used++;
    ret = 0;
__finish:
    return ret;
}

/*
des:
    拿到key对于的值
param:
    d: 字典
    key: key
return:
    成功: key对应的值
    失败: NULL
*/
void *mdbDictFetchValue(dict *d, void *key) {
    int ret = -1;
    size_t tableIdx = 0;
    unsigned int hash = 0;
    dictEntry *tmpEntry;
    if(d == NULL || key == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbDictFetchValue() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    hash = d->type->hashFunction(key);
    /* 先在 ht[0] 找 */
    tableIdx = hash & d->ht[0].mask;
    tmpEntry = d->ht[0].table[tableIdx];
    while(tmpEntry != NULL) {
        if(d->type->keyCompare(tmpEntry->key, key) == 0) {
            /* 找到了 */
            ret = 0;
            goto __finish;
        }
        tmpEntry = tmpEntry->next;
    }
    if(d->trehashidx != -1) {
        /* 正在进行rehash , 在ht[1]中找 */
        tableIdx = hash & d->ht[1].mask;
        tmpEntry = d->ht[1].table[tableIdx];
        while(tmpEntry != NULL) {
            if(d->type->keyCompare(tmpEntry->key, key) == 0) {
                /* 找到了 */
                ret = 0;
                goto __finish;
            }
            tmpEntry = tmpEntry->next;
        }    
    }
    ret = 0;
__finish:
    return ret == 0 ? (tmpEntry == NULL ? NULL : tmpEntry->val) : NULL;
}

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