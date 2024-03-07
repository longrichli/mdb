#ifndef __MDB_ALLOC_H__
#define __MDB_ALLOC_H__
#include <stdlib.h>


void *mdbMalloc(size_t sz);

void mdbFree(void *ptr);

void *mdbRealloc(void *ptr, size_t sz);

#endif /* __MDB_ALLOC_H__ */