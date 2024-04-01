#ifndef __MDB_UTIL_H__
#define __MDB_UTIL_H__
#include "mdb_sds.h"
#include "mdb_object.h"

/*
des:
    将longlong型整数转换成字符串
param:
    buf: 存放字符串的缓冲区
    len: 缓冲区大小
    val: long long 型值
return:
    转成的字符串长度
*/
int mdbLonglong2Stirng(char *numberStr, size_t len, long long val) ;

/*
des:
    string可以转换成longlong吗, 如果 llval非空, 将转换的值放入
param:
    sds: SDS
    llval: 指向存放转换出来的值的指针
return:
    转换成功: 0
    转换失败: -1
*/
int mdbIsStringRepresentableAsLongLong(SDS *sds, long long *llval);

/*
des:
    string可以转换成long吗, 如果 lval非空, 将转换的值放入
param:
    sds: SDS
    lval: 指向存放转换出来的值的指针
return:
    转换成功: 0
    转换失败: -1
*/
int mdbIsStringRepresentableAsLong(SDS *sds, long *lval);

/*
des:
    string object 可以转换成longlong吗, 如果 llval非空, 将转换的值放入
param:
    obj: 字符串对象
    llval: 指向存放转换出来的值的指针
return:
    转换成功: 0
    转换失败: -1
*/
int mdbIsObjectRepresentableAsLongLong(mobj *obj, long long *llval);

#endif /* __MDB_UTIL_H__ */