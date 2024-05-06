#include "mdb_common.h"
#include "mdb_cli_lib.h"
#include "mdb_util.h"
#include "mdb_sds.h"
#include "mdb_alloc.h"
#include <arpa/inet.h>


static int splitCmd(char *buf, char *cmd) {
    int ret = -1;
    char *subStr = NULL;
    subStr = strtok(cmd, " \t\r\n");
    while(subStr != NULL) {
        strcat(buf, subStr);
        strcat(buf, "\r\n");
        subStr = strtok(NULL, " \t\r\n");
    }
    ret = 0;
__finish:
    return ret;
}

/*
des
    连接mdb服务器
param
    host: 服务器地址
    port: 服务器端口
*/
int connectToServer(const char *host, int port) {
    int ret = -1;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "connectToServer() socket() | At %s:%d\n", __FILE__, __LINE__);
        goto __finish;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "connectToServer() connect() | At %s:%d\n", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret == 0 ? sock : -1;
}



int sendCommand(int fd, char *cmd) {
    int ret = -1;
    char buf[BIGBUFFER_SIZE] = {0};
    uint16_t cmdLen = 0, wCmdLen = 0;
    
    char *subStr = NULL;
    if(fd < 0 || cmd == NULL || cmd[0] == '\0') {
        mdbLogWrite(LOG_ERROR, "sendCommand() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    
    // 分割命令
    if(splitCmd(buf, cmd) < 0) {
        mdbLogWrite(LOG_ERROR, "sendCommand() splitCmd() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 写入长度
    cmdLen = strlen(buf);
    wCmdLen = htons(cmdLen);
    if(mdbWrite(fd, &wCmdLen, sizeof(uint16_t)) != sizeof(uint16_t)) {
        mdbLogWrite(LOG_ERROR, "sendCommand() mdbWrite() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 写入内容
    if(mdbWrite(fd, buf, cmdLen) != cmdLen) {
        mdbLogWrite(LOG_ERROR, "sendCommand() mdbWrite() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}


int readResault(int fd, char **result) {
    int ret = -1;
    uint16_t resultLen = 0, wCmdLen = 0;
    
    char *subStr = NULL;
    if(fd < 0 || result == NULL) {
        mdbLogWrite(LOG_ERROR, "readResault() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 读取长度
    if(mdbRead(fd, &resultLen, sizeof(uint16_t)) != sizeof(uint16_t)) {
        mdbLogWrite(LOG_ERROR, "readResault() mdbRead() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 字节序转换
    resultLen = ntohs(resultLen);
    // 开辟内存
    *result = mdbMalloc(resultLen + 1);
    memset(*result, 0, resultLen + 1);
    if(*result == NULL) {
        mdbLogWrite(LOG_ERROR, "readResault() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 读取内容
    if(mdbRead(fd, *result, resultLen) != resultLen) {
        mdbLogWrite(LOG_ERROR, "sendCommand() mdbRead() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    
    ret = 0;
__finish:
    return ret;
}