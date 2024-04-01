## SDS (Simple Danamic String / 简单动态字符串)
### 结构:
```c
/* SDS 结构 */
typedef struct _mdb_sds {
    size_t len;         /* buf 有效大小 */
    size_t free;        /* buf 空闲大小 */
    char buf[];         /* 存放数据的缓冲区 */
} SDS;
```
### API
```c
SDS *mdbSdsnew(char *str);
SDS *mdbSdsNewempty(void);
SDS *mdbSdscat(SDS *sds, char *str);
SDS *mdbSdscatsds(SDS *dest, SDS *src);
SDS *mdbSdsclear(SDS *sds);
void mdbSdsfree(SDS *sds);
size_t mdbSdsavail(SDS *sds);
size_t mdbSdslen(SDS *sds);



```