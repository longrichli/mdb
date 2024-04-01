#include "../src/mdb_common.h"
#include "../src/mdb_sds.h"

int main(int argc, char **argv) {
    SDS *sds = mdbSdsnew("hello world");
    SDS *sds1 = mdbSdsNewempty();
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sds->len, sds->free, sds->buf);
    sds = mdbSdsnewlen(sds, 30);
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sds->len, sds->free, sds->buf);
    sds = mdbSdscat(sds, " hi world");
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", sds->len, sds->free, sds->buf);
    sds1 = mdbSdscatsds(sds1, sds);
    mdbLogWrite(LOG_INFO, "sds1->len = %lu, sds1->free = %lu, sds1->buf = %s", sds1->len, sds1->free, sds1->buf);
    mdbLogWrite(LOG_INFO, "sds->len = %lu, sds->free = %lu, sds->buf = %s", mdbSdslen(sds), mdbSdsavail(sds), sds->buf);
    mdbSdsclear(sds1);
    mdbLogWrite(LOG_INFO, "sds1->len = %lu, sds1->free = %lu", sds1->len, sds1->free);
    mdbSdsfree(sds1);
    mdbSdsfree(sds);
    return 0;
}