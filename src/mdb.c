#include "mdb_common.h"
#include "mdb.h"
#define BANNER_FILEPATH "banner.txt"
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
int main(int argc, char **argv) {
    mdbLogWrite(LOG_INFO, "MDB server is starting 0.0.0.0:9999");
    loadBanner();
    mdbLogWrite(LOG_INFO, "Server initialized");
    mdbLogWrite(LOG_INFO, "Ready to accept connections");
    while(1) {
        sleep(3);
    }
    return 0;
}