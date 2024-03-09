#ifndef __MDB_LIST_H__
#define __MDB_LIST_H__
#include <stdlib.h>


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
    int (*match)(void *ptr, void *key); /* 用于判断两个节点的值是否相等 */
} linkedList;


/*
des:
    设置链表的dup函数
param: 
    list: 要进行设置的链表
    dup: dup函数地址
*/
void mdbListSetDupMethod(linkedList *list, void *(*dup)(void *));

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
*/
void mdbListSetFreeMethod(linkedList *list, void (*free)(void *));

/*
des:
    获取链表的free函数
param: 
    list: 要获取free函数的链表
return:
    成功: free函数地址
    失败: NULL
*/
void (*mdbListGetFreeMethod(linkedList *list))(void);

/*
des:
    设置链表的match函数
param: 
    list: 要进行设置的链表
    match: match函数地址
*/
void mdbListSetMatchMethod(linkedList *list, int (*match)(void *, void *));

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
    链表长度
*/
size_t mdbListLength(linkedList *list);

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
return:
    成功: 创建的链表
    失败: NULL
*/
linkedList *mdbListCraete(void);

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
linkedList *mdbAddNodeHead(linkedList *list, void *val);

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
linkedList *mdbAddNodeTail(void *val);


















#endif /* __MDB_LIST_H__ */