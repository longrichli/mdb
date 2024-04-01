#include "mdb_common.h"
#include "mdb_util.h"
#include <limits.h>

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
int mdbLonglong2Stirng(char *numberStr, size_t len, long long val) {
    size_t l = 0;
    char buf[32] = {0};
    char *ptr = NULL;
    unsigned long long v = 0;
    if(val == 0) return 0;
    v = val < 0 ? -val : val;
    ptr = &buf[31];
    while(v > 0) {
        *ptr-- = '0' + v % 10;
        v = v / 10;
    }
    if(val < 0) {
        *ptr-- = '-';
    }
    ptr++;
    l = 32 - (ptr - buf);
    if(l + 1 > len) {
        l = len -1;
    }
    memcpy(numberStr, buf, l);
    numberStr[l] = '\0';
    return l;
}
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
int mdbIsStringRepresentableAsLongLong(SDS *sds, long long *llval) {
    int ret = -1;
    char *endPtr = NULL;
    long long val = 0;
    char buf[32] = {0};
    size_t slen = 0;
    if(sds == NULL || sds->len == 0) {
        mdbLogWrite(LOG_ERROR, "mdbIsStringRepresentableAsLongLong() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    val = strtoll(sds->buf, &endPtr, 10);
    if(endPtr[0] != '\0') {
        goto __finish;
    }
    slen = mdbLonglong2Stirng(buf, 32, val);
    if(slen != sds->len || memcmp(buf, sds->buf, slen) != 0) {
        goto __finish;
    }
    if(llval) *llval = val;
    ret = 0;
__finish:
    return ret;
}

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
int mdbIsStringRepresentableAsLong(SDS *sds, long *lval) {
    int ret = -1;
    long long llval = 0;
    if(sds == NULL || sds->len == 0) {
        mdbLogWrite(LOG_ERROR, "mdbIsStringRepresentableAsLongLong() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(mdbIsStringRepresentableAsLongLong(sds, &llval) < 0) {
        goto __finish;
    }
    if(llval < LONG_MIN || llval > LONG_MAX) {
        goto __finish;
    }
    if(lval) *lval = llval;
    ret = 0;
__finish:
    return ret;
}

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
int mdbIsObjectRepresentableAsLongLong(mobj *obj, long long *llval) {
    int ret = -1;
    int tmpRet = -1;
    if(obj == NULL || obj->type != MDB_STRING ) {
        mdbLogWrite(LOG_ERROR, "mdbIsStringRepresentableAsLongLong() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(obj->encoding != MDB_ENCODING_RAW) {
        if(obj->encoding == MDB_ENCODING_INT) {
            if(llval) *llval = obj->ptr;
            ret = 0;
        }
        goto __finish;
    }
    tmpRet = mdbIsStringRepresentableAsLongLong((SDS *)obj->ptr, llval);
    if(tmpRet < 0) {
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

