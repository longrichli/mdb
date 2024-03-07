#ifndef __MDB_SDS_H__
#define __MDB_SDS_H__
#include <stdlib.h>
#include <sys/types.h>
/* SDS 结构 */
typedef struct _mdb_sds {
    size_t len;         /* buf 有效大小 */
    size_t free;        /* buf 空闲大小 */
    char buf[];         /* 存放数据的缓冲区 */
} SDS;

/*
des:
    创建SDS
param:
    str: SDS 字符串内容
return:
    成功: SDS
    失败: NULL
*/
SDS *newsds(char *str);

/*
des:
    创建内容为空的SDS
return:
    成功: SDS
    失败: NULL
*/
SDS *newempty(void);

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
SDS *sdsnewlen(SDS *sds, size_t newlen);

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
SDS *sdscat(SDS *sds, char *str);

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
SDS *sdscatsds(SDS *dest, SDS *src);

/*
des:
    清空SDS的内容, 但不释放内存, 清空后 len = 0
param:
    sds: 待清空的SDS
return:
    成功: 清空后的SDS
    失败: NULL
*/
SDS *sdsclear(SDS *sds);

/*
des:
    释放SDS内存
param:
    sds: 待释放的SDS
*/
void sdsfree(SDS *sds);

/*
des:
    获取SDS空闲块的大小
param:
    sds: SDS
return:
    成功: 空闲块的大小
    失败: -1
*/
ssize_t sdsavail(SDS *sds);

/*
des:
    获取SDS已分配的大小
param:
    sds: SDS
return:
    成功: 已分配的大小
    失败: -1
*/
ssize_t sdslen(SDS *sds);

#endif /* __MDB_SDS_H__ */