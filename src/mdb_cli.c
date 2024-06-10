#include "mdb_common.h"
#include "mdb_cli_lib.h"
#include "mdb_alloc.h"
#include <arpa/inet.h>

#define DEFAULT_PORT 8181
#define DEFAULT_HOST "127.0.0.1"
#define MAX_COMMAND_SIZE 8192


int isBlack(char *cmd) {
    int i = 0;
    while(cmd[i] != '\0') {
        if (cmd[i] != ' ' && cmd[i] != '\n' && cmd[i] != '\r' && cmd[i] != '\t') {
            return 0;
        }
        i++;
    }
    return 1;
}

int main(int argc, char **argv) {
    int port = DEFAULT_PORT;
    char *host = DEFAULT_HOST;
    char prefix[256] = {0};
    int dbIndex = 0;
    uint8_t code = -1;
    char *subStr = NULL;
    char *res = NULL;
    if(argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    printf("Connecting to %s:%d\n", host, port);
    int sock = connectToServer(host, port);
    if (sock == -1) {
        return 1;
    }
    // 默认进入索引为0的数据库
    sendCommand(sock, "select 0");
    readResault(sock, &res, &code);
    if(code != 0) {
        fprintf(stderr, "err: connect server failed!\n");
        disconnectFromServer(sock);
        return EXIT_FAILURE;
    }
    mdbFree(res);
    res = NULL;
    // 读取用户输入的命令
    while(1) {
        sprintf(prefix, "[mdb %s:%d(%d)]# ", host, port, dbIndex);
        printf("%s", prefix);
        char cmd[MAX_COMMAND_SIZE] = {0};
        char tmpCmd[MAX_COMMAND_SIZE] = {0};
        fgets(cmd, sizeof(cmd), stdin);
        if(isBlack(cmd)) {
            continue;
        }
        strcpy(tmpCmd, cmd);
        // 发送命令到服务器
        sendCommand(sock, cmd);
        // 读取服务器的响应
        readResault(sock, &res, &code);
        
        printf("%s", res);
        mdbFree(res);
        res = NULL;
        subStr = strtok(tmpCmd, " \n\r\t");
        if(strcmp(subStr, "exit") == 0) {
            break;
        } else if(strcmp(subStr, "select") == 0) {
            if(code == 0) {
                subStr = strtok(NULL, " \t\n\r");
                if(subStr != NULL) {
                    dbIndex = atoi(subStr);
                }
            }
        }


    }
    printf("Bye!\n");
    return 0;
}