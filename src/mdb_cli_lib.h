#ifndef __MDB_CLI_LIB__
#define __MDB_CLI_LIB__
#include <stdint.h>
/*
des
    连接mdb服务器
param
    host: 服务器地址
    port: 服务器端口
*/
int connectToServer(const char *host, int port);

int sendCommand(int fd, char *cmd);

int readResault(int fd, char **result, uint8_t* code);



#endif