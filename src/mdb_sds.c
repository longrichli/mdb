#include "mdb_sds.h"
#include "mdb_common.h"
#include "mdb_alloc.h"
#define KB (1024)
#define MB (KB * (1024))



/*
des:
    为 sds 分配新的长度, 新的长度必须大于等于原来的有效长度
param:
    sds: 待分配长度的sds
    newlen: 新的长度
return:
    成功: 分配后的sds
    失败: NULL
*/
SDS *mdbSdsnewlen(SDS *sds, size_t newlen) {
    int ret = -1;
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdsnewlen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(sds->len > newlen) {
        mdbLogWrite(LOG_ERROR, "mdbSdsnewlen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    /* 多分配一个字节用来存放 '\0' */
    sds = mdbRealloc(sds, sizeof(SDS) + newlen + 1);
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdsnewlen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    sds->free = newlen - sds->len;
    ret = 0;
__finish:
    return ret == 0 ? sds : NULL;
}

/*
des:
    创建SDS
param:
    str: SDS 字符串内容
return:
    成功: SDS
    失败: NULL
*/
SDS *mdbSdsnew(char *str) {
    int ret = -1;
    SDS *sds = NULL;
    int initlen = str == NULL ? 0 : strlen(str);
    sds = mdbMalloc(sizeof(SDS) + initlen + 1);
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdsnew() mdbMalloc() | At %s:%d",
                     __FILE__, __LINE__);
        goto __finish;
    }
    memcpy(sds->buf, str, initlen);
    sds->buf[initlen] = '\0';
    sds->len = initlen;
    sds->free = 0;
    ret = 0;
__finish:
    return ret == 0 ? sds : NULL;
}

/*
des:
    创建内容为空的SDS
return:
    成功: SDS
    失败: NULL
*/
SDS *mdbSdsNewempty(void) {
    return mdbSdsnew(NULL);
}

/*
des:
    SDS 内容拼接字符串
param:
    sds 带拼接的SDS
    str: SDS 字符串内容
return:
    成功: SDS
    失败: NULL
*/
SDS *mdbSdscat(SDS *sds, char *str) {
    int ret = -1;
    size_t newlen = 0;
    int strLen = 0;
    if(sds == NULL || str == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdscat() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    strLen = strlen(str);
    newlen = sds->len + strLen;
    /* 如果拼接后的字符串的长度小于 1MB, 那么增加同样长度的未使用空间, 如果大于 1MB, 则增加 1MB的空间 */
    if(newlen < MB) {
        sds = mdbSdsnewlen(sds, newlen << 1);
    } else {
        sds = mdbSdsnewlen(sds, newlen + MB);
    }
    memcpy(sds->buf + sds->len, str, strLen);
    sds->len = newlen;
    sds->free -= strLen; 
    sds->buf[newlen] = '\0';
    ret = 0;
__finish:
    return ret == 0 ? sds : NULL;
}

/*
des:
    SDS 内容拼接另一个SDS的内容
param:
    dest: 目标SDS
    src: 源SDS
return:
    成功: 目标 SDS
    失败: NULL
*/
SDS *mdbSdscatsds(SDS *dest, SDS *src) {
    int ret = -1;
    size_t newlen = 0;
    size_t strLen = 0;
    if(dest == NULL || src == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdscatsds() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    strLen = src->len;
    newlen = dest->len + strLen;
    /* 如果拼接后的字符串的长度小于 1MB, 那么增加同样长度的未使用空间, 如果大于 1MB, 则增加 1MB的空间 */
    if(newlen < MB) {
        dest = mdbSdsnewlen(dest, newlen << 1);
    } else {
        dest = mdbSdsnewlen(dest, newlen + MB);
    }
    memcpy(dest->buf + dest->len, src->buf, strLen);
    dest->len = newlen;
    dest->free -= strLen; 
    dest->buf[newlen] = '\0';
    ret = 0;
    
__finish:
    return ret == 0 ? dest : NULL;
}

/*
des:
    清空SDS的内容, 但不释放内存, 清空后 len = 0
param:
    sds: 待清空的SDS
return:
    成功: 清空后的SDS
    失败: NULL
*/
SDS *mdbSdsclear(SDS *sds) {
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdsclear() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    sds->free += sds->len;
    sds->len = 0;
    return sds;
}

/*
des:
    释放SDS内存
param:
    sds: 待释放的SDS
*/
void mdbSdsfree(SDS *sds) {
    mdbFree(sds);
}

/*
des:
    获取SDS空闲块的大小
param:
    sds: SDS
return:
    成功: 空闲块的大小
    失败: -1
*/
ssize_t mdbSdsavail(SDS *sds) {
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdsavail() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return sds->free;
}

/*
des:
    获取SDS已分配的大小
param:
    sds: SDS
return:
    成功: 已分配的大小
    失败: -1
*/
ssize_t mdbSdslen(SDS *sds) {
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdslen() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return sds->len;
}

/*
des:
    比较SDS
param:
    a: SDS a
    b: SDS b
return:
    a > b : >0
    a < b : <0
    a = b : =0
*/
int mdbSdsCmp(SDS *a, SDS *b) {
    if(a == NULL || b == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbSdsCmp() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return strcmp(a->buf, b->buf);
}