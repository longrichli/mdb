#ifndef __MDB_INTSET_H__
#define __MDB_INTSET_H__

#include <stdint.h>
typedef struct intset {
    uint32_t encoding;
    uint32_t length;
    int8_t contents[];
} intset;

/*
des: 
    创建一个整数集合
return:
    成功: 整数集合
    失败: NULL
*/
intset *mdbIntsetNew(void);

/*
des:
    向给定整数集合中添加一个元素
param:
    iset: 整数集合
    val: 整数值
    success: 如果添加成功, 将success指向的值置1
return:
    成功: iset的地址, 在添加过程中, iset的地址可能会改变,所以要返回
    失败: NULL
*/
intset *mdbIntsetAdd(intset *iset, int64_t val, int *success);

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
intset *mdbIntsetRemvoe(intset *iset, int64_t val, int *success);

/*
des:
    查找给定值是否在整数集合中
param:
    iset: 整数集合
    val: 整数值
return:
    存在: 1
    不存在: 0
*/
int mdbIntsetFind(intset *iset, int64_t val);

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
int mdbIntsetGet(intset *iset, uint32_t index, int64_t *val);

/*
des:
    返回整数集合中元素的数量
param:
    iset: 整数集合
return:
    整数集合中元素的个数
*/
uint32_t mdbIntsetLen(intset *iset);

/*
des:
    释放intset
param:
    iset: 整数集合
*/
void mdbIntsetFree(intset *iset);


#endif /* __MDB_INTSET_H__ */