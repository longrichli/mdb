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
    if(val == 0) {
        numberStr[0] = '0';
        numberStr[1] = '\0';
        return 1;
    }
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
    memcpy(numberStr, ptr, l);
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
            if(llval) *llval = (long long)obj->ptr;
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

/*
des:
    网络写数据
param:
    fd: 文件描述符
    data: 数据
    len: 数据长度
return:
    成功: 写入的长度
    失败: -1
*/
ssize_t mdbWrite(int fd, void *data, size_t len) {
    int ret = -1;
    size_t wAllLen = 0;
    while(len > 0) {
        ssize_t wLen = 0;
        wLen = write(fd, data, len);
        if(wLen < 0) {
            if(errno == EINTR) {
                continue;
            }
            mdbLogWrite(LOG_ERROR, "mdbWrite() write() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        } else if(wLen == 0) {
            mdbLogWrite(LOG_DEBUG, "mdbWrite() write() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        } else {
            len -= wLen;
            wAllLen += wLen;
        }
    }
    ret = 0;
__finish:
    return ret == 0 ? wAllLen : ret;
}

/*
des:
    网络读数据
param:
    fd: 文件描述符
    buf: 缓冲区
    len: 读取数据长度
return:
    成功: 读出的长度
    失败: -1
*/
ssize_t mdbRead(int fd, void *buf, size_t len) {
    int ret = -1;
    size_t rAllLen = 0;
    while(len > 0) {
        ssize_t rLen = 0;
        rLen = read(fd, buf, len);
        if(rLen < 0) {
            if(errno == EINTR) {
                continue;
            }
            mdbLogWrite(LOG_ERROR, "mdbRead() read() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        } else if(rLen == 0) {
            mdbLogWrite(LOG_DEBUG, "mdbRead() read() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        } else {
            len -= rLen;
            rAllLen += rLen;
        }
    }
    ret = 0;
__finish:
    return ret == 0 ? rAllLen : ret;
}

/*
des: 
    字符串匹配
param:
    s: 字符串
    p: 模式串
return:
    匹配: 1
    不匹配: 0
*/
int mdbStrMatch(const char* reg, const char *str) {
    int r_len = (int)strlen(reg);
    int r_p = 0;
    int r_p_last = -1;
    int s_len = (int)strlen(str);
    int s_p = 0;
    int s_p_last = -1;
 
    while (s_p < s_len) {
        if (r_p < r_len && (*(str + s_p) == *(reg + r_p))) {
            r_p++;
            s_p++;
        }
        else if (r_p < r_len && (*(reg + r_p) == '*')) {
            r_p_last = r_p;
            r_p++;
            s_p_last = s_p;
        }
        else if (r_p_last > -1) {
            r_p = r_p_last + 1;
            s_p_last++;
            s_p = s_p_last;
        }
        else {
            return 0;//false
        }
    }
    while (r_p < r_len && (*(reg + r_p) == '*')) {
        r_p++;
    }
    if (r_p == r_len) {
        return 1;//true
    }
    return 0;//false
}