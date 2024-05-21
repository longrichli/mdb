#ifndef __MDB_AOF_H__
#define __MDB_AOF_H__
#include<stdio.h>
int mdbCreateAOFFile(char *filename);
void mdbCloseAOFFile();
int mdbWriteAOFFile(char* buf, size_t sz);
ssize_t mdbReadAOFFile(char *filename, char **buf);
#endif