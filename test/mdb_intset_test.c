#include "../src/mdb_common.h"
#include "../src/mdb_intset.h"
int main(int argc, char **argv) {
    
    intset *iset = mdbIntsetNew();
    iset = mdbIntsetAdd(iset, 1, NULL);
    iset = mdbIntsetAdd(iset, 11, NULL);
    iset = mdbIntsetAdd(iset, 111, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetAdd(iset, 1111, NULL);
    iset = mdbIntsetAdd(iset, 11111, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetAdd(iset, 111111, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetAdd(iset, 1111111, NULL);
    iset = mdbIntsetAdd(iset, 11111111, NULL);
    iset = mdbIntsetAdd(iset, 111111111, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetAdd(iset, 2147483646, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    int64_t val = 2147483648;
    iset = mdbIntsetAdd(iset, val, NULL);
    iset = mdbIntsetAdd(iset, val + 10, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetRemvoe(iset, 1, NULL);
    iset = mdbIntsetRemvoe(iset, 11, NULL);
    iset = mdbIntsetRemvoe(iset, 11111, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetRemvoe(iset, 2147483646, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    iset = mdbIntsetRemvoe(iset, val + 10, NULL);
    mdbLogWrite(LOG_INFO, "iset->encoding: %u, iset->length: %u", iset->encoding, mdbIntsetLen(iset));
    
    mdbIntsetFree(iset);
    
    return 0;
}