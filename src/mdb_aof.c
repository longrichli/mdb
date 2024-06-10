#include "mdb_common.h"
#include "mdb_aof.h"
#include "mdb_util.h"
#include "mdb_alloc.h"
#define AOF_MAGIC "AOF\r\n"
#define AOF_VERSION "v1.0.0\r\n"
#define AOF_BUF_SIZE 4096
static int gAofFd = -1;

static AOFBuf *mdbResizeAOFBuf(AOFBuf *abuf) {
    size_t newLen = 0;
    if(abuf->len >= 1024 * 1024) {
        newLen = abuf->len + 1024 * 1024;
    } else {
        newLen = abuf->len * 2;
    }
    int dataSz = abuf->pos - abuf->data;
    abuf->data = mdbRealloc(abuf->data, newLen);
    if(abuf->data == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbResizeAOFBuf() mdbRealloc() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        mdbFreeAOFBuf(abuf);
        return NULL;
    }
    abuf->len = (size_t)(abuf->data) + newLen;
    abuf->pos = abuf->data + dataSz;
    return abuf;
}

static int mdbWriteAOFMagic(int fd) {
    if(write(fd, AOF_MAGIC, strlen(AOF_MAGIC)) != strlen(AOF_MAGIC)) {
        mdbLogWrite(LOG_ERROR, "mdbWriteAOFMagic() write() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

static int mdbWriteAOFVersion(int fd) {
    if(write(fd, AOF_VERSION, strlen(AOF_VERSION))!= strlen(AOF_VERSION)) {
        mdbLogWrite(LOG_ERROR, "mdbWriteAOFVersion() write() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

void mdbRenameOldAOFFile(char* filename) {
    char buf[BUFFER_SIZE] = {0};
    sprintf(buf, "%s.tmp", filename);
    rename(filename, buf);
}

AOFBuf *mdbInitAOFBuf() {
    AOFBuf *abuf = mdbMalloc(sizeof(AOFBuf));
    if(abuf == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbInitAOFBuf() zmalloc() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return NULL;
    }
    abuf->len = AOF_BUF_SIZE;
    abuf->data = mdbMalloc(AOF_BUF_SIZE);
    if(abuf->data == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbInitAOFBuf() zmalloc() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        mdbFree(abuf);
        return NULL;
    }
    abuf->pos = abuf->data;
    return abuf;
}

void mdbFreeAOFBuf(AOFBuf *abuf) {
    mdbFree(abuf->data);
    mdbFree(abuf);
}

AOFBuf *mdbAppendAOFBuf(AOFBuf *abuf, void *data, size_t len) {
    if(((size_t)abuf->pos + len) > abuf->len) {
        abuf = mdbResizeAOFBuf(abuf);
        if(abuf == NULL) {
            mdbLogWrite(LOG_ERROR, "mdbAppendAOFBuf() mdbResizeAOFBuf() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
            return NULL;
        }
    }
    memcpy(abuf->pos, data, len);
    abuf->pos += len;
    return abuf;
}
void mdbFlushAOFBuf(AOFBuf *abuf) {
    mdbWriteAOFFile(abuf->data, abuf->pos - abuf->data);
    memset(abuf->data, 0, abuf->pos - abuf->data);
    abuf->pos = abuf->data;
}

int mdbWriteAOFCommandCount(uint32_t count) {
    if(gAofFd == -1) {
        mdbLogWrite(LOG_ERROR, "mdbWriteAOFCommandCount() gAofFd == -1 | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    lseek(gAofFd, strlen(AOF_MAGIC) + strlen(AOF_VERSION), SEEK_SET);
    if(write(gAofFd, &count, sizeof(count)) != sizeof(count)) {
        mdbLogWrite(LOG_ERROR, "mdbWriteAOFCommandCount() write() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    lseek(gAofFd, 0, SEEK_END);
    return 0;
}

int mdbCreateAOFFile(char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        mdbLogWrite(LOG_ERROR, "mdbCreateAOFFile() open() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    // 写入AOF文件头
    if(mdbWriteAOFMagic(fd) < 0) {
        close(fd);
        mdbLogWrite(LOG_ERROR, "mdbCreateAOFFile() mdbWriteAOFMagic() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    // 写入AOF版本
    if(mdbWriteAOFVersion(fd) < 0) {
        close(fd);
        mdbLogWrite(LOG_ERROR, "mdbCreateAOFFile() mdbWriteAOFVersion() | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    gAofFd = fd;
    return 0;
}
void mdbCloseAOFFile() {
    if(gAofFd != -1)
        close(gAofFd);
    gAofFd = -1;
}
int mdbWriteAOFFile(char *buf, size_t len) {
    if(gAofFd == -1) {
        mdbLogWrite(LOG_ERROR, "mdbWriteAOFFile() gAofFd == -1 | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    if(mdbWrite(gAofFd, buf, len) != len) {
        mdbLogWrite(LOG_ERROR, "mdbWriteAOFFile() write() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    fsync(gAofFd);
    return 0;
}

int mdbCheckAOFMagic(int fd) {
    char magic[6] = {0};
    if(mdbRead(fd, magic, strlen(AOF_MAGIC)) != strlen(AOF_MAGIC)) {
        mdbLogWrite(LOG_ERROR, "mdbCheckAOFMagic() read() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    if(strcmp(magic, AOF_MAGIC) != 0) {
        mdbLogWrite(LOG_ERROR, "mdbCheckAOFMagic() magic != AOF_MAGIC | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return 0;

}

int mdbCheckAOFVersion(int fd) {
    char version[10] = {0};
    if(mdbRead(fd, version, strlen(AOF_VERSION)) != strlen(AOF_VERSION)) {
        mdbLogWrite(LOG_ERROR, "mdbCheckAOFVersion() read() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    if(strcmp(version, AOF_VERSION) != 0) {
        mdbLogWrite(LOG_ERROR, "mdbCheckAOFVersion() version != AOF_VERSION | At %s:%d", __FILE__, __LINE__);
        return -1;
    }
    return 0;
}
uint32_t mdbReadAOFCommandCount(int fd) {
    // 检查AOF文件头
    if(mdbCheckAOFMagic(fd) < 0) {
        mdbLogWrite(LOG_ERROR, "mdbReadAOFCommandCount() mdbCheckAOFMagic() | At %s:%d", __FILE__, __LINE__);
        return 0;
    }
    // 检查AOF版本
    if(mdbCheckAOFVersion(fd) < 0) {
        mdbLogWrite(LOG_ERROR, "mdbReadAOFCommandCount() mdbCheckAOFVersion() | At %s:%d", __FILE__, __LINE__);
        return 0;
    }
    // 读取命令数量
    uint32_t count = 0;
    if(mdbRead(fd, &count, sizeof(count)) != sizeof(count)) {
        mdbLogWrite(LOG_ERROR, "mdbReadAOFCommandCount() read() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
    }
    return count;
}
ssize_t mdbReadAOFFile(int fd, char **buf) {
    // 读取命令字节数
    uint16_t len = 0;
    ssize_t readSz = 0;
    if((readSz = mdbRead(fd, &len, sizeof(len))) != sizeof(len)) {
        mdbLogWrite(LOG_ERROR, "mdbReadAOFFile() read() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    // 读取命令内容
    *buf = mdbMalloc(len+1);
    if((readSz = mdbRead(fd, *buf, len)) != len) {
        mdbLogWrite(LOG_ERROR, "mdbReadAOFFile() read() err: %s | At %s:%d", strerror(errno), __FILE__, __LINE__);
        mdbFree(buf);
        return -1;
    }
    (*buf)[len] = '\0';
    return len;
}