#include "mdb_alloc.h"

void *mdbMalloc(size_t sz) {
    int count = 3;
    void *ptr = NULL;
    while(count-- > 0) {
        ptr = malloc(sz);
        if(ptr != NULL) {
            goto __finish;
        }
    }
__finish:
    return ptr;
}

void mdbFree(void * ptr) {
    if(ptr != NULL) {
        free(ptr);
    }
}

void *mdbRealloc(void *ptr,size_t newSize) {
    int count = 3;
    while(count-- > 0) {
        ptr = realloc(ptr, newSize);
        if(ptr != NULL) {
            goto __finish;
        }
    }
__finish:
    return ptr;
}