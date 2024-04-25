#include "mdb_common.h"
#include <arpa/inet.h>

#define DEFAULT_PORT 8181
#define DEFAULT_HOST "127.0.0.1"
#define MAX_COMMAND_SIZE 8192


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
        fprintf(stderr, "connect_to_server() socket() | At %s:%d\n", __FILE__, __LINE__);
        goto __finish;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "connect_to_server() connect() | At %s:%d\n", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret == 0 ? sock : -1;
}

int main(int argc, char **argv) {
    int port = DEFAULT_PORT;
    char *host = DEFAULT_HOST;
    char prefix[256] = {0};
    int dbIndex = 0;
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
    // 读取用户输入的命令
    while(1) {
        sprintf(prefix, "[mdb %s:%d(%d)]# ", host, port, dbIndex);
        printf("%s", prefix);
        char cmd[MAX_COMMAND_SIZE] = {0};
        fgets(cmd, sizeof(cmd), stdin);
        // 发送命令到服务器
        // 防止粘包，每次发送命令前先发送命令长度
        uint16_t cmdLen = strlen(cmd);
        write(sock, &cmdLen, sizeof(cmdLen));
        write(sock, cmd, strlen(cmd));
        // 读取服务器的响应
        // 读取响应前先读取响应长度
        uint16_t respLen = 0;
        read(sock, &respLen, sizeof(respLen));
        char resp[MAX_COMMAND_SIZE] = {0};
        read(sock, resp, respLen);
        printf("%s", resp);
        if (strcmp(cmd, "exit\n") == 0) {
            break;
        }
    }
    printf("Bye!\n");
    return 0;
}