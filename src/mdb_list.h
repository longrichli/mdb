#ifndef __MDB_LIST_H__
#define __MDB_LIST_H__
#include <stdlib.h>
#include <sys/types.h>

typedef struct listNode {
    struct listNode *pre;       /* 上一个节点 */
    struct listNode *next;      /* 下一个节点 */
    void *value;                /* 该节点的值 */
} listNode;


typedef struct linkedList {
    listNode *head;             /* 链表头节点 */
    listNode *tail;             /* 链表尾节点 */
    size_t len;                 /* 链表长度 */
    void *(*dup)(void *ptr);    /* 用于复制节点的值 */
    void (*free)(void *ptr);    /* 用于释放节点的值的内存 */
    int (*match)(void *ptr, void *key); /* 用于判断两个节点的值是否相等 相等返回 1 , 不相等返回 0*/
} linkedList;


/*
des:
    设置链表的dup函数
param: 
    list: 要进行设置的链表
    dup: dup函数地址
return: 
    成功: 0
    失败: 小于0的错误码
*/
int mdbListSetDupMethod(linkedList *list, void *(*dupMethod)(void *));

/*
des:
    获取链表的dup函数
param: 
    list: 要获取dup函数的链表
return:
    成功: dup函数地址
    失败: NULL
*/
void *(*mdbListGetDupMethod(linkedList *list))(void *);

/*
des:
    设置链表的free函数
param: 
    list: 要进行设置的链表
    free: free函数地址
return: 
    成功: 0
    失败: 小于 0 的错误码
*/
int mdbListSetFreeMethod(linkedList *list, void (*freeMethod)(void *));

/*
des:
    获取链表的free函数
param: 
    list: 要获取free函数的链表
return:
    成功: free函数地址
    失败: NULL
*/
void (*mdbListGetFreeMethod(linkedList *list))(void *);

/*
des:
    设置链表的match函数
param: 
    list: 要进行设置的链表
    match: match函数地址
return: 
    成功: 0
    失败: 小于 0 的错误码
*/
int mdbListSetMatchMethod(linkedList *list, int (*matchMethod)(void *, void *));

/*
des:
    获取链表的match函数
param: 
    list: 要获取match函数的链表
return:
    成功: match函数地址
    失败: NULL
*/
int (*mdbListGetMatchMethod(linkedList *list))(void *, void *);

/*
des:
    获取链表的长度
param:
    list: 获取长度的链表
return:
    成功: 链表长度
    失败: 小于0的错误码
*/
ssize_t mdbListLength(linkedList *list);

/*
des:
    返回链表的头节点
param:
    list: 链表
return:
    成功: 链表的头节点指针
    失败: NULL
*/
listNode *mdbListFirst(linkedList *list);

/*
des:
    返回链表的最后一个节点
param:
    list: 链表
return:
    成功: 链表的最后一个节点
    失败: NULL
*/
listNode *mdbListLast(linkedList *list);

/*
des:
    返回给定节点的前置节点
param:
    node: 给定节点
return:
    成功: 给定节点的前置节点
    失败: NULL
*/
listNode *mdbListPrevNode(listNode *node);

/*
des:
    返回给定节点的后置节点
param:
    node: 给定节点
return:
    成功: 给定节点的后置节点
    失败: NULL
*/
listNode *mdbListNextNode(listNode *node);

/*
des:
    返回给定节点的保存的值
param:
    node: 给定节点
return:
    给定节点的保存的值
*/
void *mdbListNodeValue(listNode *node);

/*
des:
    创建一个不包含任何节点的新链表
param:
    dupMethod: 复刻链表中的值的函数
    freeMethod: 释放链表中的值的函数
    mathcMethod: 查看链表中的节点的值和给定的值是否相等的函数
return:
    成功: 创建的链表
    失败: NULL
*/
linkedList *mdbListCreate(void *(*dupMethod)(void *), void(*freeMethod)(void *), int(*matchMethod)(void *, void*));

/*
des:
    将一个给定值的新节点添加到链表表头
param:
    list: 给定链表
    val: 新的节点的值
return:
    成功: 添加完之后的链表
    失败: NULL
*/
linkedList *mdbListAddNodeHead(linkedList *list, void *val);

/*
des:
    将一个给定值的新节点添加到链表表尾
param:
    list: 给定链表
    val: 新的节点的值
return:
    成功: 添加完之后的链表
    失败: NULL
*/
linkedList *mdbListAddNodeTail(linkedList *list, void *val);

/*
des:
    将一个包含给定值的新节点添加到给定节点之前或之后
param:
    list: 给定链表
    oldNode: 给定节点
    val: 新节点的值
    after: 是否在oldNode的后面插入, 否则在前面插入
return:
    成功: 插入后的链表
    失败: NULL
*/
linkedList *mdbListInsertNode(linkedList *list, listNode *oldNode, void *val, int after);

/*
des:
    查找并返回链表中包含指定值的节点
param:
    list: 链表
    val: 给定值
return:
    如果存在给定值的节点,返回此节点,否则返回 NULL
*/
listNode *mdbListSearchKey(linkedList *list, void *val);

/*
des:
    返回链表中给定索引上的节点
param:
    list: 链表
    index: 索引
return:
    成功: 指定索引的节点
    失败: NULL
*/
listNode *mdbListIndex(linkedList *list, int index);


/*
des:
    从链表中删除给定节点
param:
    list: 链表
    node: 节点
return:
    成功: 删除节点后的链表
    失败: NULL
*/
linkedList *mdbListDelNode(linkedList *list, listNode *node);

/*
des:
    复制一个给定链表的副本
param:
    给定链表
return:
    链表的副本
*/
linkedList *mdbListDup(linkedList *list);

/*
des:
    释放给定链表及其链表上的所有节点
param:
    list: 给定链表
*/
void mdbListFree(linkedList *list);




/*
des:
    修剪链表
param:
    l: 要修剪的链表
    start: 修剪的起始位置
    end: 修剪的结束位置
return:
    成功: 修剪后的链表
    失败: NULL
*/
linkedList *mdbListTrim(linkedList *l, int start, int end);















#endif /* __MDB_LIST_H__ */