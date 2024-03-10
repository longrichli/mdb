#include "../src/mdb_common.h"
#include "../src/mdb_tools.h"
#include "../src/mdb_log.h"
#include "../src/mdb_sds.h"

int main(int argc, char **argv) {

    FILE *fp = mdbCreateFile(argv[1], argv[2]);
    if(fp == NULL) {
        mdbLogWrite(LOG_ERROR, "main() mdbCreateFile()");
        exit(EXIT_FAILURE);
    }
    SDS * sds = newsds("hello world");
    fwrite(sds->buf, 1, sds->len, fp);
    sdscat(sds, "\nhi world\n");
    fwrite(sds->buf, 1, sds->len, fp);
    
    fclose(fp);
    return 0;
}