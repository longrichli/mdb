#include "mdb_common.h"
#include "mdb_alloc.h"
#include "mdb_eventloop.h"


/*
des:
    创建一个事件循环
param:
    setsize: 事件循环的最大文件描述符数
return:
    成功: 事件循环的指针
    失败: NULL
*/
mdbEventLoop *mdbCreateEventLoop(int setsize) {
    int ret = -1;
    mdbEventLoop *eventLoop = NULL;
    eventLoop = (mdbEventLoop *)mdbMalloc(sizeof(mdbEventLoop));
    if (eventLoop == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEventLoop() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
#ifdef __MAC_OS__
    eventLoop->events = (struct kevent *)mdbMalloc(sizeof(struct kevent) * setsize);
    eventLoop->changes = (struct kevent *)mdbMalloc(sizeof(struct kevent) * setsize);
    if (eventLoop->changes == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEventLoop() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        mdbFree(eventLoop);
        goto __finish;
    }
#else
    eventLoop->events = (struct epoll_event *)mdbMalloc(sizeof(struct epoll_event) * setsize);
#endif
    if (eventLoop->events == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEventLoop() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        mdbFree(eventLoop);
        goto __finish;
    }
#ifdef __MAC_OS__
    eventLoop->kq = kqueue();
    if(eventLoop->kq == -1) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEventLoop() epoll_create() | At %s:%d", __FILE__, __LINE__);
        mdbFree(eventLoop->events);
        mdbFree(eventLoop);
        goto __finish;
    }
#else
    eventLoop->epfd = epoll_create(1024);
    if (eventLoop->epfd == -1) {
        mdbLogWrite(LOG_ERROR, "mdbCreateEventLoop() epoll_create() | At %s:%d", __FILE__, __LINE__);
        mdbFree(eventLoop->events);
        mdbFree(eventLoop);
        goto __finish;
    }
#endif
    eventLoop->eventsSize = setsize;
    eventLoop->stop = 0;
    ret = 0;
__finish:
    return ret == 0 ? eventLoop : NULL;
}

/*
des:
    释放事件循环
param:
    eventLoop: 事件循环的指针
*/
void mdbDeleteEventLoop(mdbEventLoop *eventLoop) {
    if (eventLoop == NULL) {
        return;
    }
#ifdef __MAC_OS__
    mdbFree(eventLoop->changes);
#else
    close(eventLoop->epfd);
#endif
    mdbFree(eventLoop->events);
    mdbFree(eventLoop);
}

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
int mdbCreateFileEvent(mdbEventLoop *eventLoop, int fd, int mask, int (*fileProc)(mdbEventLoop *eventLoop, int fd, void *clientData, int mask), void *clientData) {
    int ret = -1;
#ifdef __MAC_OS__
    struct kevent event;
    int op = eventLoop->fileEvents[fd].mask == MDB_NONE ? EV_ADD | EV_ENABLE : EV_DELETE;
#else
    struct epoll_event ee;
    int op = eventLoop->fileEvents[fd].mask == MDB_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ee.events = 0;
#endif
    mask |= eventLoop->fileEvents[fd].mask;
    eventLoop->fileEvents[fd].mask = mask;
    eventLoop->fileEvents[fd].clientData = clientData;
#ifdef __MAC_OS__
    if(mask & MDB_READABLE) {
        EV_SET(&event, fd, EVFILT_READ, op, 0, 0, (void *)(intptr_t)fd);
        eventLoop->fileEvents[fd].rfileProc = fileProc;
        mdbLogWrite(LOG_DEBUG, "MDB_READABLE");
    }
    if(mask & MDB_WRITABLE) {
        EV_SET(&event, fd, EVFILT_WRITE, op, 0, 0, (void *)(intptr_t)fd);
        eventLoop->fileEvents[fd].wfileProc = fileProc;
        mdbLogWrite(LOG_DEBUG, "MDB_WRITEABLE");
    }
    kevent(eventLoop->kq, &event, 1, NULL, 0, NULL);
#else
    if(mask & MDB_READABLE) {
        ee.events |= EPOLLIN;
        eventLoop->fileEvents[fd].rfileProc = fileProc;
    }
    if(mask & MDB_WRITABLE) {
        ee.events |= EPOLLOUT;
        eventLoop->fileEvents[fd].wfileProc = fileProc;
    }
    ee.data.fd = fd;
    
    if(epoll_ctl(eventLoop->epfd, op, fd, &ee) == -1) {
        mdbLogWrite(LOG_ERROR, "mdbCreateFileEvent() epoll_ctl() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
#endif
    ret = 0;
__finish:
    return ret;
}

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
int mdbDeleteFileEvent(mdbEventLoop *eventLoop, int fd, int mask) {
    int ret = -1;
#ifdef __MAC_OS__
    struct kevent event;
    int newMask = eventLoop->fileEvents[fd].mask & (~mask);
    eventLoop->fileEvents[fd].mask = newMask;
    mdbLogWrite(LOG_DEBUG, "newMask= %d", newMask);
    int op = newMask == MDB_NONE ? EV_DELETE : EV_DELETE | EV_ADD;
     if(mask & MDB_READABLE) {
        EV_SET(&event, fd, EVFILT_READ, op, 0, 0, (void *)(intptr_t)fd);
    }
    if(mask & MDB_WRITABLE) {
        EV_SET(&event, fd, EVFILT_WRITE, op, 0, 0, (void *)(intptr_t)fd);
    }
    kevent(eventLoop->kq, &event, 1, NULL, 0, NULL);
#else
    struct epoll_event ee;
    int newMask = eventLoop->fileEvents[fd].mask & (~mask);
    eventLoop->fileEvents[fd].mask = newMask;
    mdbLogWrite(LOG_DEBUG, "newMask= %d", newMask);
    int op = newMask == MDB_NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    ee.events = 0;
    if(newMask & MDB_READABLE) {
        ee.events |= EPOLLIN;
    }
    if(newMask & MDB_WRITABLE) {
        ee.events |= EPOLLOUT;
    }
    ee.data.fd = fd;
    if(epoll_ctl(eventLoop->epfd, op, fd, &ee) < 0) {
        mdbLogWrite(LOG_ERROR, "mdbDeleteFileEvent() epoll_ctl() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
#endif
    ret = 0;
 __finish:
    return ret;   
}
/*
des:
    处理事件
param:
    eventLoop: 事件循环的指针
return:
    成功: 0
    失败: -1
*/
int mdbProcessEvents(mdbEventLoop *eventLoop) {
    int ret = -1;
    int fd = -1;
    if(eventLoop == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbProcessEvents() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
#ifdef __MAC_OS__
    struct timespec ts = {1, 0};
    int ready = kevent(eventLoop->kq, NULL, 0, eventLoop->events, eventLoop->eventsSize, &ts);
#else   
    int ready = epoll_wait(eventLoop->epfd, eventLoop->events, eventLoop->eventsSize, 1000);
#endif
    if(ready == -1) {
        if(errno == EINTR) {
            ret = 0;
            goto __finish;
        }

    }
    for(int i = 0; i < ready; ++i) {
#ifdef __MAC_OS__
        if(eventLoop->events[i].filter == EVFILT_READ) {
            // 调用fileevent[fd]的读回调函数
            fd = (int)(intptr_t)(eventLoop->events[i].udata);
            if ((eventLoop->fileEvents[fd]).rfileProc(eventLoop,
                                                    fd,
                                                    eventLoop->fileEvents[fd].clientData,
                                                    eventLoop->events[i].filter) < 0) {
                mdbLogWrite(LOG_DEBUG, "mdbProcessEvents() rfileProc() | At %s:%d", __FILE__, __LINE__);
            }
            mdbLogWrite(LOG_DEBUG, "kqueue: readable completed");
        }
        if(eventLoop->events[i].filter == EVFILT_WRITE) {
            fd = (int)(intptr_t)(eventLoop->events[i].udata);
            mdbLogWrite(LOG_DEBUG, "kqueue: writable");
            // 调用fileevent[fd]的写回调函数
            if ((eventLoop->fileEvents[fd]).wfileProc(eventLoop,
                                                    fd,
                                                    eventLoop->fileEvents[fd].clientData,
                                                    eventLoop->events[i].filter) < 0) {
                mdbLogWrite(LOG_DEBUG, "mdbProcessEvents() wfileProc() | At %s:%d", __FILE__, __LINE__);
            }
            mdbLogWrite(LOG_DEBUG, "kqueue: writable completed");
        }
#else
        if(eventLoop->events[i].events == EPOLLIN) {
            // 调用fileevent[fd]的读回调函数
            fd = eventLoop->events[i].data.fd;
            if((eventLoop->fileEvents[fd]).rfileProc(eventLoop,
                                                 fd, 
                                                 eventLoop->fileEvents[fd].clientData,
                                                 eventLoop->events[i].events) < 0) {
                mdbLogWrite(LOG_DEBUG, "mdbProcessEvents() rfileProc() | At %s:%d", __FILE__, __LINE__);
            }

        }
        if(eventLoop->events[i].events == EPOLLOUT) {
            // 调用fileevent[fd]的写回调函数
            fd = eventLoop->events[i].data.fd;
            if((eventLoop->fileEvents[fd]).wfileProc(eventLoop,
                                                 fd, 
                                                 eventLoop->fileEvents[fd].clientData,
                                                 eventLoop->events[i].events) < 0) {
                mdbLogWrite(LOG_DEBUG, "mdbProcessEvents() wfileProc() | At %s:%d", __FILE__, __LINE__);
            }
        }
        if(eventLoop->events[i].events == EPOLLHUP || eventLoop->events[i].events == EPOLLERR) {
            mdbLogWrite(LOG_DEBUG, "mdbProcessEvents() epoll_wait() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
#endif 
    }
   
    ret = 0;
__finish:
    return ret;
}

/*
des:
    启动事件循环
param:
    eventLoop: 事件循环的指针
return:
    成功: 0
    失败: -1
*/
int mdbStartEventLoop(mdbEventLoop *eventLoop) {
    int ret = -1;
    while(!eventLoop->stop) {
        if(mdbProcessEvents(eventLoop) < 0) {
            mdbLogWrite(LOG_ERROR, "mdbStartEventLoop() mdbProcessEvents() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }
    }
    ret = 0;
__finish:
    return ret;
}

/*
des:
    停止事件循环
param:
    eventLoop: 事件循环实例
return:
    成功: 0
    失败: -1
*/
void mdbStopEventLoop(mdbEventLoop *eventLoop) {
    eventLoop->stop = 1;
}