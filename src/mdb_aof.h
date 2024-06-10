#ifndef __MDB_AOF_H__
#define __MDB_AOF_H__
#include<stdio.h>
#define AOF_DEFAULT_PATH "./mdb.aof"
#define AOF_DEFAULT_FLUSH_TIME (1)
typedef struct AOFBuf {
    char *data;
    char *pos;
    size_t len;
} AOFBuf;


void mdbRenameOldAOFFile(char* filename);

AOFBuf *mdbInitAOFBuf();
void mdbFreeAOFBuf(AOFBuf *abuf);
int mdbWriteAOFCommandCount(uint32_t count);
AOFBuf *mdbAppendAOFBuf(AOFBuf *abuf, void *data, size_t len);
void mdbFlushAOFBuf(AOFBuf *abuf);
int mdbCreateAOFFile(char *filename);
void mdbCloseAOFFile();
int mdbWriteAOFFile(char *buf, size_t len);
uint32_t mdbReadAOFCommandCount(int fd);
ssize_t mdbReadAOFFile(int fd, char **buf);
#endif