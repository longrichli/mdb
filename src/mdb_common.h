#ifndef __MDB_COMMON_H__
#define __MDB_COMMON_H__

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include "mdb_log.h"
#include "mdb_tools.h"
extern char *strsignal (int __sig) __THROW;

#define MINBUFFER_SIZE (256)
#define BUFFER_SIZE (4096)
#define BIGBUFFER_SIZE (8192)

#define TEST

#endif /* __MDB_COMMON_H__ */