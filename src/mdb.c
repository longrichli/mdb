#include "mdb_common.h"
#include "mdb.h"
#include "mdb_eventloop.h"
#include <getopt.h>
#include <arpa/inet.h>
#define BANNER_FILEPATH "banner.txt"
#define DEFAULT_PORT 8181
#define DEFAULT_IP "127.0.0.1"
#define LOG_PATH "mdb.log"
#define DEFAULT_LOG_LEVEL LOG_INFO

void loadBanner(void) {
    FILE *fp = fopen(BANNER_FILEPATH, "r");
    char buf[BUFFER_SIZE] = {0};
    if(fp == NULL) {
        mdbLogWrite(LOG_ERROR, "loadBanner() fopen() | At %s:%d", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    fread(buf, sizeof(char), BUFFER_SIZE, fp);
    fwrite(buf, sizeof(char), strlen(buf), stdout);
    fclose(fp);
    
}

/*
des
    后台运行
return
    成功: 0
    失败: -1
*/
int daemonize(void) {
    int ret = -1;
    pid_t pid = fork();
    if(pid < 0) {
        goto __finish;
    }
    if(pid > 0) {
        exit(EXIT_SUCCESS);
    }
    if(setsid() == -1) {
        goto __finish;
    }
    if(chdir("/") == -1) {
        goto __finish;
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    ret = 0;
__finish:
    return ret;
}


/*
des:
    处理客户端读事件
param:

return:
    成功: 0
    失败: -1
*/
int clientFileProc(mdbEventLoop *eventLoop, int fd, void *clientData, int mask) {
    uint8_t buf[BUFFER_SIZE] = {0};
    uint16_t dataLen = 0;
    if(read(fd, &dataLen, sizeof(dataLen)) <= 0) {
        mdbDeleteFileEvent(eventLoop, fd, MDB_READABLE);
        close(fd);
        return 0;
    }
    mdbLogWrite(LOG_DEBUG, "1111");
    dataLen = ntohs(dataLen);
    if(read(fd, buf, dataLen) <= 0) {
        mdbDeleteFileEvent(eventLoop, fd, MDB_READABLE);
        close(fd);
        return 0;
    }
    mdbLogWrite(LOG_DEBUG, "2222");
    dataLen = htons(dataLen);
    if(write(fd, &dataLen, sizeof(dataLen)) < 0) {
        mdbDeleteFileEvent(eventLoop, fd, MDB_READABLE);
        close(fd);
        return 0;
    }
     mdbLogWrite(LOG_DEBUG, "333");
    if(write(fd, buf, strlen(buf)) < 0) {
        mdbDeleteFileEvent(eventLoop, fd, MDB_READABLE);
        close(fd);
        return 0;
    }
     mdbLogWrite(LOG_DEBUG, "444");
    return 0;
    
}
/*
des:
    接收客户端描述符, 将接收到的客户端描述符放入事件循环
param:

return:
    成功: 0
    失败: -1
*/
int acceptFileProc(mdbEventLoop *eventLoop, int fd, void *clientData, int mask) {
    int ret = -1;
    struct sockaddr_in clientSock;
    memset(&clientSock, 0, sizeof(clientSock));
    socklen_t sockLen = 0;
    int clientFd = accept(fd, (struct sockaddr *)&clientSock, &sockLen);
    if(clientFd < 0) {
        mdbLogWrite(LOG_ERROR, "acceptFileProc() accept() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 打印客户端ip地址
    char* ip = inet_ntoa(clientSock.sin_addr);
    mdbLogWrite(LOG_INFO, "client ip: %s", ip);
    // 添加文件事件
    if(mdbCreateFileEvent(eventLoop, clientFd, MDB_READABLE, clientFileProc, NULL) < 0) {
        mdbLogWrite(LOG_ERROR, "acceptFileProc() mdbCreateFileEvent() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0; 
__finish:
    return ret;
}


/*
des
    开启服务
param
    ip: ip地址
    port: 端口
return
    成功: 0
    失败: -1    
*/
int startService(const char *ip, int port) {
    int ret = -1;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int clientfd = -1;
    if(listenfd == -1) {
        mdbLogWrite(LOG_ERROR, "startService() socket() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    struct sockaddr_in cliaddr;
    socklen_t cliaddrLen = 0;

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        mdbLogWrite(LOG_ERROR, "startService() bind() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(listen(listenfd, 1024) == -1) {
        mdbLogWrite(LOG_ERROR, "startService() listen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    mdbLogWrite(LOG_INFO, "Server start at %s:%d", ip, port);

    mdbEventLoop *loop = mdbCreateEventLoop(1024);
    mdbLogWrite(LOG_DEBUG, "create EventLoop Completed!");
    if(loop == NULL) {
        mdbLogWrite(LOG_ERROR, "startService() mdbCreateEventLoop()  | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }

     // 添加文件事件
    if(mdbCreateFileEvent(loop, listenfd, MDB_READABLE, acceptFileProc, NULL) < 0) {
        mdbLogWrite(LOG_ERROR, "startService() mdbCreateFileEvent() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    mdbLogWrite(LOG_DEBUG, "create FileEvnet Completed!");
    if(mdbStartEventLoop(loop) < 0) {
        mdbLogWrite(LOG_ERROR, "startService() mdbStartEventLoop() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

int main(int argc, char **argv) {
    int port = DEFAULT_PORT;
    char ip[16] = {0};
    // 默认后台运行
    int daemon = 1;
    logLevel level = DEFAULT_LOG_LEVEL;
    strncpy(ip, DEFAULT_IP, sizeof(ip));
    int opt;
    while ((opt = getopt(argc, argv, "i:p:d:hl")) != -1) {
        switch (opt) {
            case 'i':
                strncpy(ip, optarg, sizeof(ip));
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                if(strcmp(optarg, "1") == 0) {
                    daemon = 1;
                } else {
                    daemon = 0;
                }
                break;
            case 'l':
                level = atoi(optarg);
                break;
            case 'h':
                mdbLogWrite(LOG_INFO, "Usage: %s [-i ip] [-p port] [-d]", argv[0]);
                exit(EXIT_SUCCESS);
            default:
                mdbLogWrite(LOG_ERROR, "Usage: %s [-i ip] [-p port] [-d]", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    loadBanner();
    if(daemon) {
        mdbLogWrite(LOG_INFO, "Daemonizing...");
        if(daemonize() == -1) {
            mdbLogWrite(LOG_ERROR, "daemonize() failed | At %s:%d", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        mdbLogInit(level, LOG_PATH);
    }

    int listenfd = startService(ip, port);
    if(listenfd == -1) {
        mdbLogWrite(LOG_ERROR, "startService() failed | At %s:%d", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    return 0;
}