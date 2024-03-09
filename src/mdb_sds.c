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
SDS *sdsnewlen(SDS *sds, size_t newlen) {
    int ret = -1;
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "sdsnewlen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(sds->len > newlen) {
        mdbLogWrite(LOG_ERROR, "sdsnewlen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    /* 多分配一个字节用来存放 '\0' */
    sds = mdbRealloc(sds, sizeof(SDS) + newlen + 1);
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "sdsnewlen() | At %s:%d", __FILE__, __LINE__);
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
SDS *newsds(char *str) {
    int ret = -1;
    SDS *sds = NULL;
    int initlen = str == NULL ? 0 : strlen(str);
    sds = mdbMalloc(sizeof(SDS) + initlen + 1);
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "newsds() mdbMalloc() | At %s:%d",
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
SDS *newempty(void) {
    return newsds(NULL);
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
SDS *sdscat(SDS *sds, char *str) {
    int ret = -1;
    size_t newlen = 0;
    int strLen = 0;
    if(sds == NULL || str == NULL) {
        mdbLogWrite(LOG_ERROR, "sdscat() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    strLen = strlen(str);
    newlen = sds->len + strLen;
    /* 如果拼接后的字符串的长度小于 1MB, 那么增加同样长度的未使用空间, 如果大于 1MB, 则增加 1MB的空间 */
    if(newlen < MB) {
        sds = sdsnewlen(sds, newlen << 1);
    } else {
        sds = sdsnewlen(sds, newlen + MB);
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
SDS *sdscatsds(SDS *dest, SDS *src) {
    int ret = -1;
    size_t newlen = 0;
    size_t strLen = 0;
    if(dest == NULL || src == NULL) {
        mdbLogWrite(LOG_ERROR, "sdscatsds() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    strLen = src->len;
    newlen = dest->len + strLen;
    /* 如果拼接后的字符串的长度小于 1MB, 那么增加同样长度的未使用空间, 如果大于 1MB, 则增加 1MB的空间 */
    if(newlen < MB) {
        dest = sdsnewlen(dest, newlen << 1);
    } else {
        dest = sdsnewlen(dest, newlen + MB);
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
SDS *sdsclear(SDS *sds) {
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "sdsclear() | At %s:%d", __FILE__, __LINE__);
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
void sdsfree(SDS *sds) {
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
ssize_t sdsavail(SDS *sds) {
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "sdsavail() | At %s:%d", __FILE__, __LINE__);
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
ssize_t sdslen(SDS *sds) {
    if(sds == NULL) {
        mdbLogWrite(LOG_ERROR, "sdslen() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return sds->len;
}