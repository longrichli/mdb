#include "mdb_common.h"
#include "mdb_cli_lib.h"
#include "mdb_alloc.h"
#include <arpa/inet.h>

#define DEFAULT_PORT 8181
#define DEFAULT_HOST "127.0.0.1"
#define MAX_COMMAND_SIZE 8192




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
        char *res = NULL;
        fgets(cmd, sizeof(cmd), stdin);
        // 发送命令到服务器
        sendCommand(sock, cmd);
        // 读取服务器的响应
        readResault(sock, &res);
        printf("%s", res);
        mdbFree(res);
        if (strcmp(cmd, "exit") == 0) {
            break;
        }
    }
    printf("Bye!\n");
    return 0;
}