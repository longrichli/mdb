#include "../src/mdb_common.h"
#include "../src/mdb_sds.h"

int main(int argc, char **argv) {
    SDS *sds = newsds("hello world");
    SDS *sds1 = newempty();
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sds->len, sds->free, sds->buf);
    sds = sdsnewlen(sds, 30);
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sds->len, sds->free, sds->buf);
    sds = sdscat(sds, " hi world");
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sds->len, sds->free, sds->buf);
    sds1 = sdscatsds(sds1, sds);
    mdbLogWrite(LOG_INFO, "sds1->len = %lu, sds1->free = %lu, sds1->buf = %s", sds1->len, sds1->free, sds1->buf);
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sdslen(sds), sdsavail(sds), sds->buf);
    sdsclear(sds1);
    mdbLogWrite(LOG_INFO, "sds1->len = %lu, sds1->free = %lu", sds1->len, sds1->free);
    sdsfree(sds1);
    sdsfree(sds);
    return 0;
}