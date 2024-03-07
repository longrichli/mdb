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
SDS *newsds(char *str);
SDS *newempty(void);
SDS *sdscat(SDS *sds, char *str);
SDS *sdscatsds(SDS *dest, SDS *src);
SDS *sdsclear(SDS *sds);
void sdsfree(SDS *sds);
size_t sdsavail(SDS *sds);
size_t sdslen(SDS *sds);



```