#ifndef __MDB_EVENTLOOP_H__
#define __MDB_EVENTLOOP_H__

#include <sys/epoll.h>

#define MDB_READABLE EPOLLIN
#define MDB_WRITABLE EPOLLOUT
#define MDB_NONE (0)
#define MDB_MAX_FILE_EVENTS ((1024) * (10))
typedef struct mdbEventLoop mdbEventLoop;
typedef struct fileEvent {
    int mask;
    void *clientData;
    int (*rfileProc)(struct mdbEventLoop *eventLoop, int fd, void *clientData, int mask);
    int (*wfileProc)(struct mdbEventLoop *eventLoop, int fd, void *clientData, int mask);
} fileEvent;

typedef struct mdbEventLoop {
    int epfd;
    struct epoll_event *events;
    fileEvent fileEvents[MDB_MAX_FILE_EVENTS];
    int eventsSize;
    int stop;
} mdbEventLoop;

/*
des:
    创建一个事件循环
param:
    setsize: 事件循环的最大文件描述符数
return:
    成功: 事件循环的指针
    失败: NULL
*/
mdbEventLoop *mdbCreateEventLoop(int setsize);

/*
des:
    释放事件循环
param:
    eventLoop: 事件循环的指针
*/
void mdbDeleteEventLoop(mdbEventLoop *eventLoop);

/*
des:
    创建一个文件事件
param:
    eventLoop: 事件循环的指针
    fd: 文件描述符
    mask: 事件掩码
    fileProc: 文件事件处理函数
    clientData: 文件事件处理函数的参数
return:
    成功: 0
    失败: -1
*/
int mdbCreateFileEvent(mdbEventLoop *eventLoop, int fd, int mask, int (*fileProc)(mdbEventLoop *eventLoop, int fd, void *clientData, int mask), void *clientData);

/*
des:
    删除一个文件事件
param:
    eventLoop: 事件循环的指针
    fd: 文件描述符
    mask: 事件掩码
return:
    成功: 0
    失败: -1
*/
int mdbDeleteFileEvent(mdbEventLoop *eventLoop, int fd, int mask);
/*
des:
    处理事件
param:
    eventLoop: 事件循环的指针
return:
    成功: 0
    失败: -1
*/
int mdbProcessEvents(mdbEventLoop *eventLoop);

/*
des:
    启动事件循环
param:
    eventLoop: 事件循环的指针
return:
    成功: 0
    失败: -1
*/
int mdbStartEventLoop(mdbEventLoop *eventLoop);


/*
des:
    停止事件循环
param:
    eventLoop: 事件循环实例
return:
    成功: 0
    失败: -1
*/
void mdbStopEventLoop(mdbEventLoop *eventLoop);
#endif  /* __MDB_EVENTLOOP_H__ */