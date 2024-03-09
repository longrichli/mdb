# list 链表
## 结构
```c
typedef struct listNode {
    struct listNode *pre;       /* 上一个节点 */
    struct listNode *next;      /* 下一个节点 */
    void *value;                /* 该节点的值 */
} listNode;


typedef struct list {
    listNode *head;             /* 链表头节点 */
    listNode *tail              /* 链表尾节点 */
    size_t len;                 /* 链表长度 */
    void *(*dup)(void *ptr);    /* 用于复制节点的值 */
    void (*free)(void *ptr);    /* 用于释放节点的值的内存 */
    int (*match)(void *ptr, void *key); /* 用于判断两个节点的值是否相等 */
} list;
```

