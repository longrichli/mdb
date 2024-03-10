#include <sys/stat.h>
#include "mdb_tools.h"
#include "mdb_common.h"

/*
des:
    如果目录不存在,创建目录
param:
    dirname: 目录名
return:
    成功: 0
    失败: 小于 0 的错误码
*/
static int createDir(const char *dirname) {
    int tmpRet = -1;
    int ret = -1;
    struct stat sa;
    memset(&sa, 0, sizeof(sa));
    if((tmpRet = stat(dirname, &sa)) < 0) {
        if(errno == ENOENT) {
            if((tmpRet = mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) < 0) {
                mdbLogWrite(LOG_ERROR, "createDir() mkdir() errcode[%d] | At %s:%d", tmpRet, __FILE__, __LINE__);
                goto __finish;
            }
        } else {
                mdbLogWrite(LOG_ERROR, "createDir() stat() errcode[%d] | At %s:%d", tmpRet, __FILE__, __LINE__);
                goto __finish;
        }
    } 
    ret = 0;
__finish:
    return ret;
}

/*
des:
    根据指定的mode创建文件
param:
    filename: 文件名
    mode: 创建模式: 
        r	打开一个已有的文本文件，允许读取文件。
        w	打开一个文本文件，允许写入文件。如果文件不存在，
            则会创建一个新文件。在这里，您的程序会从文件的
            开头写入内容。如果文件存在，则该会被截断为零长度，
            重新写入。
        a	打开一个文本文件，以追加模式写入文件。如果文件不
            存在，则会创建一个新文件。在这里，您的程序会在已
            有的文件内容中追加内容。
        r+	打开一个文本文件，允许读写文件。
        w+	打开一个文本文件，允许读写文件。如果文件已存在，
            则文件会被截断为零长度，如果文件不存在，则会创建
            一个新文件。
        a+	打开一个文本文件，允许读写文件。如果文件不存在，
        则会创建一个新文件。读取会从文件的开头开始，写入则
        只能是追加模式。
return:
    成功: 文件指针
    失败: NULL
*/
static FILE *createFile(const char *filename, const char *mode) {
    FILE *fp = NULL;
    if(filename == NULL || filename[0] == '\0' || mode == NULL || mode[0] == '\0') {
        mdbLogWrite(LOG_ERROR, "mdbCreateFile() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    if((fp = fopen(filename, mode)) == NULL) {
        mdbLogWrite(LOG_ERROR, "createFile() fopen() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    return fp;
}

/*
des:
    根据指定的mode创建文件, 文件名可以存在多级目录, 如: /root/a/b/c.txt
param:
    filename: 文件名
    mode: 创建模式: 
        r	打开一个已有的文本文件，允许读取文件。
        w	打开一个文本文件，允许写入文件。如果文件不存在，
            则会创建一个新文件。在这里，您的程序会从文件的
            开头写入内容。如果文件存在，则该会被截断为零长度，
            重新写入。
        a	打开一个文本文件，以追加模式写入文件。如果文件不
            存在，则会创建一个新文件。在这里，您的程序会在已
            有的文件内容中追加内容。
        r+	打开一个文本文件，允许读写文件。
        w+	打开一个文本文件，允许读写文件。如果文件已存在，
            则文件会被截断为零长度，如果文件不存在，则会创建
            一个新文件。
        a+	打开一个文本文件，允许读写文件。如果文件不存在，
        则会创建一个新文件。读取会从文件的开头开始，写入则
        只能是追加模式。
        如果处理的是二进制文件，则需使用下面的访问模式来取代上面的访问模式：
        "rb", "wb", "ab", "rb+", "r+b", "wb+", "w+b", "ab+", "a+b"
return:
    成功: 文件指针
    失败: NULL
*/
FILE *mdbCreateFile(const char *filename, const char *mode) {
    int ret = -1;
    int tmpRet = -1;
    char *curToken = NULL;
    char *preToken = NULL;
    char cwd[FILENAME_MAX] = {0};
    FILE *fp = NULL;
    char *cwdPtr = cwd;
    char filenameDup[FILENAME_MAX] = {0};
    if(filename == NULL || filename[0] == '\0' || mode == NULL || mode[0] == '\0') {
        mdbLogWrite(LOG_ERROR, "mdbCreateFile() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    /* 保存当前目录 , 因为后面创建多级目录时, 改变工作目录, 所以为了回来, 先保存起来 */
    cwdPtr = getcwd(cwd, FILENAME_MAX);
    if(cwdPtr == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateFile() getcwd() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    /* 使用filename 副本进行操作 */
    strcpy(filenameDup, filename);
    /* 如果第一个字符就是 '/', 说明是从跟目录开始, 应该切换到根目录进行创建 */
    if(filenameDup[0] == '/' || filenameDup[0] == '\\') {
        chdir("/");
    }
    preToken = strtok(filenameDup, "/\\");
    while(preToken != NULL) {
        curToken = strtok(NULL, "/\\");
        if(curToken == NULL) {
            /* 说明preToken是一个文件, 进行创建文件, 如果文件已经存在, 避免覆盖重要数据, 不进行创建, 返回错误 */
            if((fp = createFile(preToken, mode)) == NULL) {
                mdbLogWrite(LOG_ERROR, "mdbCreateFile() createFile() | At %s:%d",
                             __FILE__, __LINE__);
                goto __finish;
            }
        } else {
            /* 说明preToken是一个目录, 进行创建目录, 如果目录存在, 什么也不做 */
            if((tmpRet = createDir(preToken)) < 0) {
                mdbLogWrite(LOG_ERROR, "mdbCreateFile() createDir() errcode[%d] | At %s:%d",
                            tmpRet, __FILE__, __LINE__);
                goto __finish;
            }
            chdir(preToken);
        }
        preToken = curToken;
    }

    /* 切换回开始的工作目录*/
    chdir(cwdPtr);
    ret = 0;
__finish:
    return ret == 0 ? fp : NULL;
}
