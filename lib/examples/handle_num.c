#include <stdio.h>
#include "../../include/mdb_cli_lib.h"
#include <malloc.h>
#include <stdlib.h>
int main() {
    char *result = NULL;
    uint8_t code = 0;
    int serverFd = connectToServer("127.0.0.1", 8181);
    if(serverFd < 0) {
        fprintf(stderr, "Err: Connect MDB Server failed\n");
        exit(EXIT_FAILURE);
    }
    sendCommand(serverFd, "select 0");
    readResault(serverFd, &result, &code);
    free(result);
    result = NULL;
    if(code != 0) {
        fprintf(stderr, "Err: Connect MDB Server failed\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < 500; i++) {
        int random = rand() & 0xffff;
        if(random < 0xffff * 0.3) {
            sendCommand(serverFd, "decrby num 51");
            printf("decrby num 51\n");
        } else if(random < 0xffff * 0.6){
            sendCommand(serverFd, "incr num");
            printf("incr num\n");
        } else {
            sendCommand(serverFd, "decrby num 50");
            printf("decrby num 50\n");
        } 
        readResault(serverFd, &result, &code);
        free(result);
    }
    disconnectFromServer(serverFd);
    printf("exit\n");
    return 0;
}