#include "mdb_common.h"
#include "mdb_list.h"
#include "mdb_alloc.h"

/*
des:
    设置链表的dup函数
param: 
    list: 要进行设置的链表
    dup: dup函数地址
return: 
    成功: 0
    失败: 小于 0 的错误码
*/
int mdbListSetDupMethod(linkedList *list, void *(*dupMethod)(void *)) {
    if(list == NULL || dupMethod == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListSetDupMethod() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    list->dup = dupMethod;
    return 0;
}

/*
des:
    获取链表的dup函数
param: 
    list: 要获取dup函数的链表
return:
    成功: dup函数地址
    失败: NULL
*/
void *(*mdbListGetDupMethod(linkedList *list))(void *) {
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListGetDupMethod() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return list->dup;
}

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
int mdbListSetFreeMethod(linkedList *list, void (*freeMethod)(void *)) {
    if(list == NULL || freeMethod == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListSetFreeMethod() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    list->free = freeMethod;
    return 0;
}

/*
des:
    获取链表的free函数
param: 
    list: 要获取free函数的链表
return:
    成功: free函数地址
    失败: NULL
*/
void (*mdbListGetFreeMethod(linkedList *list))(void *) {
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListGetFreeMethod() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return list->free;
}

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
int mdbListSetMatchMethod(linkedList *list, int (*matchMethod)(void *, void *)) {
    if(list == NULL || matchMethod == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListSetMatchMethod() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    list->match = matchMethod;
    return 0;
}

/*
des:
    获取链表的match函数
param: 
    list: 要获取match函数的链表
return:
    成功: match函数地址
    失败: NULL
*/
int (*mdbListGetMatchMethod(linkedList *list))(void *, void *) {
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListGetMatchMethod() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return list->match;
}

/*
des:
    获取链表的长度
param:
    list: 获取长度的链表
return:
    成功: 链表长度
    失败: 小于0的错误码
*/
ssize_t mdbListLength(linkedList *list) {
     if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListLength() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return list->len;
}

/*
des:
    返回链表的头节点
param:
    list: 链表
return:
    成功: 链表的头节点指针
    失败: NULL
*/
listNode *mdbListFirst(linkedList *list) {
     if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListFirst() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return list->head;
}

/*
des:
    返回链表的最后一个节点
param:
    list: 链表
return:
    成功: 链表的最后一个节点
    失败: NULL
*/
listNode *mdbListLast(linkedList *list) {
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListLast() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return list->tail;
}

/*
des:
    返回给定节点的前置节点
param:
    node: 给定节点
return:
    成功: 给定节点的前置节点
    失败: NULL
*/
listNode *mdbListPrevNode(listNode *node) {
    if(node != NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListPrevNode() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return node->pre;
}

/*
des:
    返回给定节点的后置节点
param:
    node: 给定节点
return:
    成功: 给定节点的后置节点
    失败: NULL
*/
listNode *mdbListNextNode(listNode *node) {
    if(node != NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListNextNode() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return node->next;
}

/*
des:
    返回给定节点的保存的值
param:
    node: 给定节点
return:
    成功: 给定节点的保存的值
    失败: NULL
*/
void *mdbListNodeValue(listNode *node) {
    if(node != NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListNodeValue() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return node->value;
}

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
linkedList *mdbListCreate(void *(*dupMethod)(void *), void(*freeMethod)(void *), int(*matchMethod)(void *, void*)) {
    int ret = -1;
    linkedList *list = NULL;
    if(freeMethod == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if((list = mdbMalloc(sizeof(linkedList))) == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListCreate() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    list->dup = dupMethod;
    list->free = freeMethod;
    list->match = matchMethod;
    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
    ret = 0;
__finish:
    return ret == 0 ? list : NULL;
}

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
linkedList *mdbAddNodeHead(linkedList *list, void *val) {
    int ret = -1;
    listNode *node = NULL;
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbAddNodeHead() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    node = mdbMalloc(sizeof(*node));
    if(node == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbAddNodeHead() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    node->value = val;
    if(list->len == 0) {
        list->head = node;
        list->tail = node;
        node->next = NULL;
        node->pre = NULL;
    } else {
        node->pre = NULL;
        node->next = list->head;
        list->head->pre = node;
        list->head = node;
    }
    list->len++;
    ret = 0;
__finish:
    return ret == 0 ? list : NULL;
}

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
linkedList *mdbAddNodeTail(linkedList *list, void *val) {
    int ret = -1;
    listNode *node = NULL;
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbAddNodeTail() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    node = mdbMalloc(sizeof(*node));
    if(node == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbAddNodeTail() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    node->value = val;
    if(list->len == 0) {
        list->head = node;
        list->tail = node;
        node->next = NULL;
        node->pre = NULL;
    } else {
        node->pre = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    ret = 0;
__finish:
    return ret == 0 ? list : NULL;
}

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
linkedList *mdbListInsertNode(linkedList *list, listNode *oldNode, void *val, int after) {
    int ret = -1;
    listNode *node = NULL;
    if(list == NULL || oldNode == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListInsertNode() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    node = mdbMalloc(sizeof(*node));
    if(node == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbAddNodeTail() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    node->value = val;
    if(after) {
        if(oldNode == list->tail) {
            node->pre = list->tail;
            node->next = NULL;
            list->tail->next = node;
            list->tail = node;
        } else {
            node->next = oldNode->next;
            oldNode->next->pre = node;
            oldNode->next = node;
            node->pre = oldNode;
        }   
    } else {
        if(oldNode == list->head) {
            node->pre = NULL;
            node->next = list->head;
            list->head->pre = node;
            list->head = node;
        } else {
            node->next = oldNode;
            node->pre = oldNode->pre;
            oldNode->pre->next = node;
            oldNode->pre = node;
        }
    }
    list->len++;
    ret = 0;
__finish:
    return ret == 0 ? list : NULL;
}

/*
des:
    查找并返回链表中包含指定值的节点
param:
    list: 链表
    val: 给定值
return:
    如果存在给定值的节点,返回此节点,否则返回 NULL
*/
listNode *mdbListSearchKey(linkedList *list, void *val) {
    listNode *pNode = NULL;
    if(list == NULL || val == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListSearchKey() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    pNode = list->head;
    while(pNode != NULL) {
        if(list->match(pNode->value, val)) {
            return pNode;
        }
        pNode = pNode->next;
    }
    return NULL;
}

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
listNode *mdbListIndex(linkedList *list, int index) {
    listNode *node = NULL;
    int middle = 0;
    if(list == NULL || index < 0 || index >= list->len) {
        mdbLogWrite(LOG_ERROR, "mdbListIndex() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    /* 算一下中间位置 */
    middle = list->len / 2;
    if(index <= middle) {
        node = list->head;
        for(int i = 0; i < index; i++) {
            node = node->next;
        }
    } else {
        node = list->tail;
        for(int i = list->len - 1; i > index; i--) {
            node = node->pre;
        }
    }
    return node;
}


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
linkedList *mdbListDelNode(linkedList *list, listNode *node) {
    int ret = -1;
    if(list == NULL || node == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListInsertNode() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(node == list->head && node == list->tail) {
        list->head == NULL;
        list->tail == NULL;
    } else if(node == list->tail) {
        list->tail = node->pre;
        node->pre->next = NULL;
       
    } else if(node == list->head) {
        list->head = node->next;
        node->next->pre = NULL;
    } else {
        node->pre->next = node->next;
        node->next->pre = node->pre;
    }
    list->free(node->value);
    mdbFree(node);
    list->len--;
    ret = 0;
__finish:
    return ret == 0 ? list : NULL;
}

/*
des:
    复制一个给定链表的副本
param:
    给定链表
return:
    链表的副本
*/
linkedList *mdbListDup(linkedList *list) {
    int ret = -1;
    linkedList *listDup = NULL;
    listNode *pNode = NULL;
    
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListDup() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if((listDup = mdbListCreate(list->dup, list->free, list->match)) == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListDup() mdbListCreate() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    pNode = list->head;
    while(pNode != NULL) {
        void *val = list->dup(pNode->value);
        mdbAddNodeTail(listDup, val);
        pNode = pNode->next;
    }
    ret = 0;
__finish:
    return ret == 0 ? listDup : NULL;
}

/*
des:
    释放给定链表及其链表上的所有节点
param:
    list: 给定链表
*/
void mdbListFree(linkedList *list) {
    listNode *pNode = NULL;
    if(list == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbListFree() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    pNode = list->tail;
    /* 将节点一个个删掉 */
    while(pNode != NULL) {
        listNode *tmpNode = pNode->pre;
        mdbListDelNode(list, pNode);
        pNode = tmpNode;
    }
    mdbFree(list);
}