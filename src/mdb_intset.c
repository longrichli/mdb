#include "mdb_common.h"
#include "mdb_alloc.h"
#include "mdb_intset.h"

#define INTSET_ENC_INT16 (sizeof(int16_t))
#define INTSET_ENC_INT32 (sizeof(int32_t))
#define INTSET_ENC_INT64 (sizeof(int64_t))

static uint8_t intsetValEncoding(int64_t val) {
    if(val < INT32_MIN || val > INT32_MAX) {
        return INTSET_ENC_INT64;
    } else if(val < INT16_MIN || val > INT16_MAX) {
        return INTSET_ENC_INT32;
    }
    return INTSET_ENC_INT16;
}

static intset *intsetResize(intset *iset, uint32_t newLen) {
    int ret = -1;
    iset = mdbRealloc(iset, sizeof(*iset) + newLen * iset->encoding);
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "intsetResize() mdbRealloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret == 0 ? iset : NULL;
}

static void intsetSet(intset *iset, uint32_t pos, int64_t val) {
    if(iset->encoding == INTSET_ENC_INT64) {
        ((int64_t *)(iset->contents))[pos] = (int64_t)val;
    } else if(iset->encoding == INTSET_ENC_INT32) {
        ((int32_t *)(iset->contents))[pos] = (int32_t)val;
    } else {
        ((int16_t *)(iset->contents))[pos] = (int16_t)val;
    }
}

static int64_t intsetGetEncoded(intset *iset, uint32_t pos, uint32_t encoding) {
    if(encoding == INTSET_ENC_INT64) {
        return ((int64_t *)(iset->contents))[pos];
    } else if(encoding == INTSET_ENC_INT32) {
        return ((int32_t *)(iset->contents))[pos];
    } else {
        return ((int16_t *)(iset->contents))[pos];
    }
}

static int64_t intsetGet(intset *iset, uint32_t pos) {
    return intsetGetEncoded(iset, pos, iset->encoding);
}

static intset *intsetUpgradeAndAdd(intset *iset, int64_t val, int *success) {
    int curenc = iset->encoding;
    int newenc = intsetValEncoding(val);
    // 如果val小于0, 说明它比所有的数都小, 将会放在最前面, 如果大于0, 说明它比所有的数大, 放在最后面, //prepend 时 pre 和 pend 的缩写意为前后
    int prepend = val < 0 ? 1 : 0;
    int length = iset->length;
    iset->encoding = newenc; 
    iset = intsetResize(iset, iset->length + 1);
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "intsetUpgradeAndAdd() intsetReize() | At %s:%d", __FILE__, __LINE__);
        if(success) *success = 0;
        goto __finish;
    }
    while(length-- > 0) {
        intsetSet(iset, iset->length + prepend, intsetGetEncoded(iset, iset->length, curenc));
    }
    if(prepend) {
        intsetSet(iset, 0, val);
    } else {
        intsetSet(iset, iset->length, val);
    }
    // 长度 + 1
    iset->length++;
    if(success) *success = 1;
__finish:
    return iset;
}

/*
des:
    再intset中查找给定值, 如果存在, 将下标放到pos指向的地址中,返回 1, 否则返回 0
param:
    iset: iset指针
    val: 查找的值
    pos: 下标会存放再pos指定的地址
return:
    存在: 1
    不存在: 0
*/
static int intsetSearch(intset *iset, int64_t val, uint32_t *pos) {
    int left = 0;
    int right = iset->length - 1;
    int mid = -1;
    int ret = 0;
    int64_t cur = 0;
    if(iset->length == 0) {
        if(pos) *pos = 0;
        goto __finish;
    } else if(val < intsetGet(iset, 0)) {
        if(pos) *pos = 0;
        mdbLogWrite(LOG_DEBUG, "val < intsetGet(iset, 0)");
        goto __finish;
    } else if(val > intsetGet(iset, iset->length - 1)) {
        if(pos) *pos = iset->length;
        mdbLogWrite(LOG_DEBUG, "val > intsetGet(iset, iset->length - 1)");
        goto __finish;
    }

    while(right >= left) {
        mid = left + (right - left) / 2;
        cur = intsetGet(iset, mid);
        if(val < cur) {
            right = mid - 1;
        } else if(val > cur) {
            left = mid + 1;
        } else {
            break;
        }
    }
    if(val == cur) {
        //找到了
        if (pos) *pos = mid;
    } else {
        if (pos) *pos = left;
    }

__finish:
    mdbLogWrite(LOG_DEBUG, "val= %ld, cur= %ld", val, cur);
    return ret;
}

static void intsetMoveTail(intset *is, uint32_t from, uint32_t to) {
    void *src, *dst;
    uint32_t bytes = is->length-from;
    if (is->encoding == INTSET_ENC_INT64) {
        src = (int64_t*)is->contents+from;
        dst = (int64_t*)is->contents+to;
        bytes *= sizeof(int64_t);
    } else if (is->encoding == INTSET_ENC_INT32) {
        src = (int32_t*)is->contents+from;
        dst = (int32_t*)is->contents+to;
        bytes *= sizeof(int32_t);
    } else {
        src = (int16_t*)is->contents+from;
        dst = (int16_t*)is->contents+to;
        bytes *= sizeof(int16_t);
    }
    memmove(dst,src,bytes);
}

/*
des: 
    创建一个整数集合
return:
    成功: 整数集合
    失败: NULL
*/
intset *mdbIntsetNew(void) {
    int ret = -1;
    intset *iset = NULL;
    iset = mdbMalloc(sizeof(*iset));
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    iset->encoding = INTSET_ENC_INT16;
    iset->length = 0;
    ret = 0;
__finish:
    return ret == 0 ? iset : NULL;
}

/*
des:
    像给定整数集合中添加一个元素
param:
    iset: 整数集合
    val: 整数值
    success: 如果添加成功, 将success指向的值置1
return:
    成功: iset的地址, 在添加过程中, iset的地址可能会改变,所以要返回
    失败: NULL
*/
intset *mdbIntsetAdd(intset *iset, int64_t val, int *success) {
    uint8_t encoding = 0;
    uint32_t pos = 0; 
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbIntsetAdd() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    encoding = intsetValEncoding(val);
    if(encoding > iset->encoding) {
        // 需要升级了
        iset = intsetUpgradeAndAdd(iset, val, success);
        if(iset == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbIntsetAdd() intsetUpgradeAndAdd() | At %s:%d", __FILE__, __LINE__);
            if(success) *success = 0;
            goto __finish;
        }
    } else {
        // 先看看是否存在这个值
        if(intsetSearch(iset, val, &pos)) {
            mdbLogWrite(LOG_ERROR, "mdbIntsetAdd() error: intset exist the value. | At %s:%d", __FILE__, __LINE__);
            if(success) *success = 0;
            goto __finish;
        }
        // 没有这个值, 就进行扩容添加
        iset = intsetResize(iset, iset->length + 1);
        if(iset == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbIntsetAdd() intsetResize() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
        // 空出位置用来存放val
        if(pos < iset->length) {
            intsetMoveTail(iset, pos, pos + 1);
        }
        intsetSet(iset, pos, val);
        //长度 + 1
        iset->length++;
        if(success) *success = 1;
    }
__finish:
    return iset;
}

/*
des:
    移除给定整数集合的给定值
param:
    iset: 整数集合
    val: 整数值
    success: 如果移除成功, 将success指向的值置1
reutrn:
    成功: iset的地址, 在移除过程中, iset的地址可能会改变,所以要返回
    失败: NULL
*/
intset *mdbIntsetRemvoe(intset *iset, int64_t val, int *success) {
    uint32_t pos = 0;
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbIntsetRemvoe() | At %s:%d", __FILE__, __LINE__);
        if(success) *success = 0;
        goto __finish;
    }
    uint32_t valenc = intsetValEncoding(val);
    if(valenc <= iset->encoding && intsetSearch(iset, val, &pos)) {
        if(pos < iset->length - 1) {
            intsetMoveTail(iset, pos + 1, pos);
        }
        iset = intsetResize(iset, iset->length - 1);
        if(iset == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbIntsetRemvoe() intsetResize() | At %s:%d", __FILE__, __LINE__);
            if(success) *success = 0;
            goto __finish;
        }
        iset->length--;
        if(success) *success = 1;
    } else {
        mdbLogWrite(LOG_ERROR, "mdbIntsetRemvoe() error: Not found the val in iset | At %s:%d", __FILE__, __LINE__);
        if(success) *success = 0;
        goto __finish;
    }
__finish:
    return iset;
}

/*
des:
    查找给定值是否在整数集合中
param:
    iset: 整数集合
    val: 整数值
return:
    存在: 1
    不存在: 0
    错误: -1;
*/
int mdbIntsetFind(intset *iset, int64_t val) {
    int ret = -1;
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbIntsetFind() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    uint32_t valenc = intsetValEncoding(val);
    if(valenc <= iset->encoding && intsetSearch(iset, val, NULL)) {
        ret = 1;
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

/*
des:
    返回整数集合中给定索引上的值
param:
    iset: 整数集合
    index: 索引
    val: 用于存放值的地址
return:
    成功: 0 
    失败: -1 (当索引超出范围时, 返回-1)
*/
int mdbIntsetGet(intset *iset, uint32_t index, int64_t *val) {
    int ret = -1;
    if(iset == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbIntsetFind() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(index >= iset->length) {
        mdbLogWrite(LOG_ERROR, "mdbIntsetFind() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    *val = intsetGet(iset, index);
    ret = 0;
__finish:
    return ret;
}

/*
des:
    返回整数集合中元素的数量
param:
    iset: 整数集合
return:
    整数集合中元素的个数
*/
uint32_t mdbIntsetLen(intset *iset) {
    return iset->length;
}

/*
des:
    释放intset
param:
    iset: 整数集合
*/
void mdbIntsetFree(intset *iset) {
    if(iset != NULL) {
        mdbFree(iset);
    }
}