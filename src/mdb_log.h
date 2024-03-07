#ifndef __MDB_LOG_H__
#define __MDB_LOG_H__

typedef enum _mdb_log_level {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
} logLevel;

void mdbLogInit(logLevel level, char *filename);
void mdbLogWrite(logLevel level, char *fmt, ...);


#endif /* __MDB_LOG_H__ */