#include "mdb_common.h"
#include "mdb.h"
#include "mdb_eventloop.h"
#include "mdb_list.h"
#include "mdb_alloc.h"
#include "mdb_object.h"
#include <getopt.h>
#include <arpa/inet.h>
#include "mdb_config.h"
#include "mdb_util.h"
#define BANNER_FILEPATH "banner.txt"
#define DEFAULT_PORT (8181)
#define DEFAULT_IP "127.0.0.1"
#define LOG_PATH "mdb.log"
#define DEFAULT_LOG_LEVEL LOG_INFO
#define DEFAULT_DB_SIZE (16)
#define CONFIG_FILEPATH "../config/mdb.conf"
mdbServer gServer;



// 通用命令

// SELECT：切换到指定的数据库。
// 例如：SELECT db_index
void mdbCommandSelect(mdbClient *c) {
    int fd = c->fd;
    long dbIndex = -1;
    if(c->argc != 2) {
        // 返回错误信息
        if(mdbSendReply(fd, "ERR wrong number of arguments\r\n", MDB_REP_ERROR) == -1) {
            mdbLogWrite(LOG_ERROR, "mdbCommandSelect() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    if(mdbIsStringRepresentableAsLong((SDS *)(c->argv[1]->ptr), &dbIndex) < 0) {
        // 返回错误信息
        if(mdbSendReply(fd, "ERR invalid db index\r\n", MDB_REP_ERROR) < 0) {
            mdbLogWrite(LOG_ERROR, "mdbCommandSelect() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    if(dbIndex < 0 || dbIndex >= gServer.dbnum) {
        // 返回错误信息
        if(mdbSendReply(fd, "ERR invalid db index\r\n", MDB_REP_ERROR) < 0) {
            mdbLogWrite(LOG_ERROR, "mdbCommandSelect() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            return;
        }
        return;
    }
    c->db = &gServer.db[dbIndex];
    // 回复OK
    if(mdbSendReply(fd, "OK\r\n", MDB_REP_OK) < 0) {
        mdbLogWrite(LOG_ERROR, "mdbCommandSelect() mdbSendReply() | At %s:%d", __FILE__, __LINE__);
        return;
    }
}

// KEYS：用于查找满足指定模式的键。
// 例如：KEYS *、KEYS user:*
void mdbCommandKeys(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        // 返回错误信息
        mdbSendReply(fd, "ERR wrong number of arguments for 'keys' command\r\n", MDB_REP_ERROR);
        return;
    }
    mobj **keys = (mobj **)mdbDictAllKey(c->db->dict);
    if(keys == NULL) {
        // 返回空数组
        mdbSendReply(fd, "empty array\r\n", MDB_REP_NIL);
        return;
    }
    SDS *reply = mdbSdsNewempty();
    for(int i = 0; i < mdbDictSize(c->db->dict); i++) {
        mobj *key = keys[i];
        int r = mdbStrMatch(((SDS *)(c->argv[1]->ptr))->buf, ((SDS *)(key->ptr))->buf);
        if(r == 1) {
            mdbLogWrite(LOG_DEBUG, "match key: %s", ((SDS *)(key->ptr))->buf);
            reply = mdbSdscat(reply, ((SDS *)(key->ptr))->buf);
            reply = mdbSdscat(reply, "\r\n");
        }
    }
    if(reply->len == 0 || reply->buf[0] == '\0') {
        // 返回空数组
        mdbSendReply(fd, "empty array\r\n", MDB_REP_NIL);
    } else {
        mdbSendReply(fd, reply->buf, MDB_REP_ARRAY);
    }
    
    mdbSdsfree(reply);
    mdbFree(keys);
    
}
// DEL：用于删除一个或多个键。
// 例如：DEL key1 key2
void mdbCommandDel(mdbClient *c) {
    int fd = c->fd;
    if(c->argc < 2) {
        mdbSendReply(fd, "ERR wrong number of arguments for 'del' command\r\n", MDB_REP_STRING);
        return;
    }
    int count = 0;
    for(int i = 1; i < c->argc; i++) {
        if(mdbDictFetchValue(c->db->dict, c->argv[i]) != NULL) {
            mdbDictDelete(c->db->dict, c->argv[i]);
            count++;
        }
    }
    char buf[32] = {0};
    sprintf(buf, "%d\r\n", count);
    mdbSendReply(fd, buf, MDB_REP_STRING);

}
// EXISTS：用于检查指定键是否存在。
// 例如：EXISTS key
void mdbCommandExists(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        mdbSendReply(fd, "ERR wrong number of arguments for 'exists' command\r\n", MDB_REP_STRING);
        return;
    }
    mobj *obj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(obj == NULL) {
        mdbSendReply(fd, "0\r\n", MDB_REP_STRING);
    } else {
        mdbSendReply(fd, "1\r\n", MDB_REP_STRING);
    }
}
// TYPE：用于获取指定键的数据类型。
// 例如：TYPE key
void mdbCommandType(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 2) {
        mdbSendReply(fd, "ERR wrong number of arguments for 'type' command\r\n", MDB_REP_STRING);
        return;
    }
    mobj *obj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(obj == NULL) {
        mdbSendReply(fd, "none\r\n", MDB_REP_STRING);
        return;
    }
    char buf[32] = {0};
    char *type = mdbObjectType(obj);
    sprintf(buf, "%s\r\n", type);
    mdbSendReply(fd, buf, MDB_REP_STRING);
}
// RENAME：用于重命名一个键。
// 例如：RENAME old_key new_key
void mdbCommandRename(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR wrong number of arguments for 'rename' command\r\n", MDB_REP_STRING);
        return;
    }
    mobj *obj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(obj == NULL) {
        mdbSendReply(fd, "ERR no such key\r\n", MDB_REP_STRING);
        return;
    }
    mobj *key = mdbDupStringObject(c->argv[2]);
    mobj *val = mdbDupStringObject(obj);
    mdbDictDelete(c->db->dict, c->argv[1]);
    mdbDictReplace(c->db->dict, key, val);
    mdbSendReply(fd, "OK\r\n", MDB_REP_OK);
}
// RENAMEX：用于重命名一个键，仅在新键不存在时执行。
// 例如：RENAMEX old_key new_key
void mdbCommandRenamex(mdbClient *c) {
    int fd = c->fd;
    if(c->argc != 3) {
        mdbSendReply(fd, "ERR wrong number of arguments for'renamex' command\r\n", MDB_REP_STRING);
        return;
    }
    if(mdbDictFetchValue(c->db->dict, c->argv[2]) != NULL) {
        mdbSendReply(fd, "ERR new key already existed\r\n", MDB_REP_STRING);
        return;
    }
    mobj *obj = mdbDictFetchValue(c->db->dict, c->argv[1]);
    if(obj == NULL) {
        mdbSendReply(fd, "ERR no such key\r\n", MDB_REP_STRING);
        return;
    }
    mobj *key = mdbDupStringObject(c->argv[2]);
    mobj *val = mdbDupStringObject(obj);
    mdbDictDelete(c->db->dict, c->argv[1]);
    mdbDictAdd(c->db->dict, key, val);
    mdbSendReply(fd, "OK\r\n", MDB_REP_OK);
}
// EXPIRE：设置键的过期时间（以秒为单位）。
// 例如：EXPIRE key seconds
void mdbCommandExpire(mdbClient *c) {
    mdbSendReply(c->fd, "ERR not support command 'expire' yet\r\n", MDB_REP_ERROR);
}
// TTL：获取键的剩余过期时间（以秒为单位）。
// 例如：TTL key
void mdbCommandTtl(mdbClient *c) {
    mdbSendReply(c->fd, "ERR not support command 'ttl' yet\r\n", MDB_REP_ERROR);
}
// PERSIST：移除键的过期时间，使其永不过期。
// 例如：PERSIST key
void mdbCommandPersist(mdbClient *c) {
    mdbSendReply(c->fd, "ERR not support command 'persist' yet\r\n", MDB_REP_ERROR);
}
// SCAN：迭代数据库中的键。
// 例如：SCAN cursor [MATCH pattern] [COUNT count]
void mdbCommandScan(mdbClient *c) {
    mdbSendReply(c->fd, "ERR not support command 'scan' yet\r\n", MDB_REP_ERROR);
}

mdbCommand *mdbCreateCmd(SDS *name, cmdProc *proc) {
    mdbCommand *cmd = mdbMalloc(sizeof(mdbCommand));
    if(cmd == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateCmd() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    cmd->name = name;
    cmd->proc = proc;
    return cmd;
}

void mdbFreeCmd(void *cmd) {
    if(cmd == NULL) return;
    // 在dirc中已经被释放
    // mdbSdsfree(cmd->name);
    mdbFree(cmd);
}

unsigned int mdbCmdSdsHash(const void *sds) {
    return mdbBkdrHash(((SDS *)sds)->buf);
}

int mdbCmdSdsKeyCompare(const void *key1, const void *key2) {
    int l1,l2;
    l1 = mdbSdslen((SDS *)key1);
    l2 = mdbSdslen((SDS *)key2);
    mdbLogWrite(LOG_DEBUG, "mdbCmdSdsKeyCompare() key1 = %s, key2 = %s", ((SDS *)key1)->buf, ((SDS *)key2)->buf);
    if (l1 != l2) return 0;
    return memcmp(((SDS *)key1)->buf, ((SDS *)key2)->buf, l1) == 0;
}
void mdbCmdSdsFree(void *ptr) {
    mdbSdsfree((SDS *)ptr);
}




dictType gCmdDtype = {
    mdbCmdSdsHash,        // hash函数
    NULL,                 // key复制函数
    NULL,                 // value复制函数
    mdbSdsKeyCompare,  // key比较函数
    mdbCmdSdsFree,        // key析构函数
    mdbFreeCmd            // value析构函数
};


void initCommandDict() {
    
    gServer.mdbCommands = mdbDictCreate(&gCmdDtype);
    if(gServer.mdbCommands == NULL) {
        mdbLogWrite(LOG_ERROR, "initCommandDict() mdbDictCreate() | At %s:%d", __FILE__, __LINE__);
        return;
    }
    //添加通用命令
    mdbCommand *select = mdbCreateCmd(mdbSdsnew("select"), mdbCommandSelect);
    mdbDictAdd(gServer.mdbCommands, select->name, select);
    mdbCommand *keys = mdbCreateCmd(mdbSdsnew("keys"), mdbCommandKeys);
    mdbDictAdd(gServer.mdbCommands, keys->name, keys);
    mdbCommand *del = mdbCreateCmd(mdbSdsnew("del"), mdbCommandDel);
    mdbDictAdd(gServer.mdbCommands, del->name, del);
    mdbCommand *exists = mdbCreateCmd(mdbSdsnew("exists"), mdbCommandExists);
    mdbDictAdd(gServer.mdbCommands, exists->name, exists);
    mdbCommand *type = mdbCreateCmd(mdbSdsnew("type"), mdbCommandType);
    mdbDictAdd(gServer.mdbCommands, type->name, type);
    mdbCommand *rename = mdbCreateCmd(mdbSdsnew("rename"), mdbCommandRename);
    mdbDictAdd(gServer.mdbCommands, rename->name, rename);
    mdbCommand *renamex = mdbCreateCmd(mdbSdsnew("renamex"), mdbCommandRenamex);
    mdbDictAdd(gServer.mdbCommands, renamex->name, renamex);
    mdbCommand *expire = mdbCreateCmd(mdbSdsnew("expire"), mdbCommandExpire);
    mdbDictAdd(gServer.mdbCommands, expire->name, expire);
    mdbCommand *ttl = mdbCreateCmd(mdbSdsnew("ttl"), mdbCommandTtl);
    mdbDictAdd(gServer.mdbCommands, ttl->name, ttl);
    mdbCommand *persist = mdbCreateCmd(mdbSdsnew("persist"), mdbCommandPersist);
    mdbDictAdd(gServer.mdbCommands, persist->name, persist);
    mdbCommand *scan = mdbCreateCmd(mdbSdsnew("scan"), mdbCommandScan);
    mdbDictAdd(gServer.mdbCommands, scan->name, scan);

    // 字符串相关命令
    mdbCommand *set = mdbCreateCmd(mdbSdsnew("set"), mdbCommandSet);
    mdbDictAdd(gServer.mdbCommands, set->name, set);
    mdbCommand *get = mdbCreateCmd(mdbSdsnew("get"), mdbCommandGet);
    mdbDictAdd(gServer.mdbCommands, get->name, get);
    mdbCommand *append = mdbCreateCmd(mdbSdsnew("append"), mdbCommandAppend);
    mdbDictAdd(gServer.mdbCommands, append->name, append);
    mdbCommand *incrby = mdbCreateCmd(mdbSdsnew("incrby"), mdbCommandIncrby);
    mdbDictAdd(gServer.mdbCommands, incrby->name, incrby);
    mdbCommand *decrby = mdbCreateCmd(mdbSdsnew("decrby"), mdbCommandDecrby);
    mdbDictAdd(gServer.mdbCommands, decrby->name, decrby);
    mdbCommand *incr = mdbCreateCmd(mdbSdsnew("incr"), mdbCommandIncr);
    mdbDictAdd(gServer.mdbCommands, incr->name, incr);
    mdbCommand *strLen = mdbCreateCmd(mdbSdsnew("strlen"), mdbCommandStrlen);
    mdbDictAdd(gServer.mdbCommands, strLen->name, strLen);
    mdbCommand *setrange = mdbCreateCmd(mdbSdsnew("setrange"), mdbCommandSetrange);
    mdbDictAdd(gServer.mdbCommands, setrange->name, setrange);
    mdbCommand *getrange = mdbCreateCmd(mdbSdsnew("getrange"), mdbCommandGetrange);
    mdbDictAdd(gServer.mdbCommands, getrange->name, getrange);

    // 列表相关命令
    mdbCommand *lpush = mdbCreateCmd(mdbSdsnew("lpush"), mdbCommandLpush);
    mdbDictAdd(gServer.mdbCommands, lpush->name, lpush);
    mdbCommand *rpush = mdbCreateCmd(mdbSdsnew("rpush"), mdbCommandRpush);
    mdbDictAdd(gServer.mdbCommands, rpush->name, rpush);
    mdbCommand *lpop = mdbCreateCmd(mdbSdsnew("lpop"), mdbCommandLpop);
    mdbDictAdd(gServer.mdbCommands, lpop->name, lpop);
    mdbCommand *rpop = mdbCreateCmd(mdbSdsnew("rpop"), mdbCommandRpop);
    mdbDictAdd(gServer.mdbCommands, rpop->name, rpop);
    mdbCommand *lindex = mdbCreateCmd(mdbSdsnew("lindex"), mdbCommandLindex);
    mdbDictAdd(gServer.mdbCommands, lindex->name, lindex);
    mdbCommand *llen = mdbCreateCmd(mdbSdsnew("llen"), mdbCommandLlen);
    mdbDictAdd(gServer.mdbCommands, llen->name, llen);
    mdbCommand *linsert = mdbCreateCmd(mdbSdsnew("linsert"), mdbCommandLinsert);
    mdbDictAdd(gServer.mdbCommands, linsert->name, linsert);
    mdbCommand *lrem = mdbCreateCmd(mdbSdsnew("lrem"), mdbCommandLrem);
    mdbDictAdd(gServer.mdbCommands, lrem->name, lrem);
    mdbCommand *ltrim = mdbCreateCmd(mdbSdsnew("ltrim"), mdbCommandLtrim);
    mdbDictAdd(gServer.mdbCommands, ltrim->name, ltrim);
    mdbCommand *lset = mdbCreateCmd(mdbSdsnew("lset"), mdbCommandLset);
    mdbDictAdd(gServer.mdbCommands, lset->name, lset);
    mdbCommand *lrange = mdbCreateCmd(mdbSdsnew("lrange"), mdbCommandLrange);
    mdbDictAdd(gServer.mdbCommands, lrange->name, lrange);

    // 哈希相关命令
    mdbCommand *hset = mdbCreateCmd(mdbSdsnew("hset"), mdbCommandHset);
    mdbDictAdd(gServer.mdbCommands, hset->name, hset);
    mdbCommand *hget = mdbCreateCmd(mdbSdsnew("hget"), mdbCommandHget);
    mdbDictAdd(gServer.mdbCommands, hget->name, hget);
    mdbCommand *hexsits = mdbCreateCmd(mdbSdsnew("hexsits"), mdbCommandHexists);
    mdbDictAdd(gServer.mdbCommands, hexsits->name, hexsits);
    mdbCommand *hdel = mdbCreateCmd(mdbSdsnew("hdel"), mdbCommandHdel);
    mdbDictAdd(gServer.mdbCommands, hdel->name, hdel);
    mdbCommand *hlen = mdbCreateCmd(mdbSdsnew("hlen"), mdbCommandHlen);
    mdbDictAdd(gServer.mdbCommands, hlen->name, hlen);
    mdbCommand *hkeys = mdbCreateCmd(mdbSdsnew("hkeys"), mdbCommandHkeys);
    mdbDictAdd(gServer.mdbCommands, hkeys->name, hkeys);
    mdbCommand *hgetall = mdbCreateCmd(mdbSdsnew("hgetall"), mdbCommandHgetall);
    mdbDictAdd(gServer.mdbCommands, hgetall->name, hgetall);

    // 集合相关命令
    mdbCommand *sadd = mdbCreateCmd(mdbSdsnew("sadd"), mdbCommandSadd);
    mdbDictAdd(gServer.mdbCommands, sadd->name, sadd);
    mdbCommand *scard = mdbCreateCmd(mdbSdsnew("scard"), mdbCommandScard);
    mdbDictAdd(gServer.mdbCommands, scard->name, scard);
    mdbCommand *sismember = mdbCreateCmd(mdbSdsnew("sismember"), mdbCommandSismember);
    mdbDictAdd(gServer.mdbCommands, sismember->name, sismember);
    mdbCommand *smembers = mdbCreateCmd(mdbSdsnew("smembers"), mdbCommandSmembers);
    mdbDictAdd(gServer.mdbCommands, smembers->name, smembers);
    mdbCommand *srandmember = mdbCreateCmd(mdbSdsnew("srandmember"), mdbCommandSrandmember);
    mdbDictAdd(gServer.mdbCommands, srandmember->name, srandmember);
    mdbCommand *spop = mdbCreateCmd(mdbSdsnew("spop"), mdbCommandSpop);
    mdbDictAdd(gServer.mdbCommands, spop->name, spop);
    mdbCommand *srem = mdbCreateCmd(mdbSdsnew("srem"), mdbCommandSrem);
    mdbDictAdd(gServer.mdbCommands, srem->name, srem);

    // 有序集合相关命令
    mdbCommand *zadd = mdbCreateCmd(mdbSdsnew("zadd"), mdbCommandZadd);
    mdbDictAdd(gServer.mdbCommands, zadd->name, zadd);
    mdbCommand *zcard = mdbCreateCmd(mdbSdsnew("zcard"), mdbCommandZcard);
    mdbDictAdd(gServer.mdbCommands, zcard->name, zcard);
    mdbCommand *zcount = mdbCreateCmd(mdbSdsnew("zcount"), mdbCommandZcount);
    mdbDictAdd(gServer.mdbCommands, zcount->name, zcount);
    mdbCommand *zrange = mdbCreateCmd(mdbSdsnew("zrange"), mdbCommandZrange);
    mdbDictAdd(gServer.mdbCommands, zrange->name, zrange);
    mdbCommand *zrevrange = mdbCreateCmd(mdbSdsnew("zrevrange"), mdbCommandZrevrange);
    mdbDictAdd(gServer.mdbCommands, zrevrange->name, zrevrange);
    mdbCommand *zrank = mdbCreateCmd(mdbSdsnew("zrank"), mdbCommandZrank);
    mdbDictAdd(gServer.mdbCommands, zrank->name, zrank);
    mdbCommand *zrevrank = mdbCreateCmd(mdbSdsnew("zrevrank"), mdbCommandZrevrank);
    mdbDictAdd(gServer.mdbCommands, zrevrank->name, zrevrank);
    mdbCommand *zrem = mdbCreateCmd(mdbSdsnew("zrem"), mdbCommandZrem);
    mdbDictAdd(gServer.mdbCommands, zrem->name, zrem);
    mdbCommand *zscore = mdbCreateCmd(mdbSdsnew("zscore"), mdbCommandZscore);
    mdbDictAdd(gServer.mdbCommands, zscore->name, zscore);
}

void loadConfig() {
// 加载配置文件
    if(mdbLoadConfig(CONFIG_FILEPATH) >= 0) {
        // 获取配置
        dict *config = mdbGetConfig();
        if(config != NULL) {
            SDS *ipkey = mdbSdsnew("ip");
            SDS *ipval = mdbDictFetchValue(config, ipkey);
            if(ipval != NULL) {
                memset(gServer.ip, 0, sizeof(gServer.ip));
                strncpy(gServer.ip, ipval->buf, ipval->len);
                mdbLogWrite(LOG_DEBUG, "ip: %s", gServer.ip);
            }
            mdbSdsfree(ipkey);
            SDS *portkey = mdbSdsnew("port");
            SDS *portval = mdbDictFetchValue(config, portkey);
            if(portval != NULL) {
                gServer.port = atoi(portval->buf);
                mdbLogWrite(LOG_DEBUG, "port: %d", gServer.port);
            }
            mdbSdsfree(portkey);
            SDS *logpathKey = mdbSdsnew("logpath");
            SDS *logpathVal = mdbDictFetchValue(config, logpathKey);
            if(logpathVal != NULL) {
                memset(gServer.logpath, 0, sizeof(gServer.logpath));
                strncpy(gServer.logpath, logpathVal->buf, logpathVal->len);
                mdbLogWrite(LOG_DEBUG, "logpath: %s", gServer.logpath);
            }
            mdbSdsfree(logpathKey);
            SDS *loglevelKey = mdbSdsnew("loglevel");
            SDS *loglevelVal = mdbDictFetchValue(config, loglevelKey);
            if(loglevelVal != NULL) {
                if(strcmp(loglevelVal->buf, "debug") == 0) {
                    gServer.loglevel = LOG_DEBUG;
                } else if(strcmp(loglevelVal->buf, "info") == 0) {
                    gServer.loglevel = LOG_INFO;
                } else if(strcmp(loglevelVal->buf, "error") == 0) {
                    gServer.loglevel = LOG_ERROR;
                } else if(strcmp(loglevelVal->buf, "warning") == 0) {
                    gServer.loglevel = LOG_WARNING;
                } else {
                    gServer.loglevel = LOG_INFO;
                }
            }
            mdbSdsfree(loglevelKey);
            SDS *databasesKey = mdbSdsnew("databases");
            SDS *databasesVal = mdbDictFetchValue(config, databasesKey);
            if(databasesVal != NULL) {
                gServer.dbnum = atoi(databasesVal->buf);
            }
            mdbSdsfree(databasesKey);
            mdbDictFree(config);

        }

    }
}

dictType gDBDtype = {
    mdbHashFun,           // hash function
    NULL,                 // keydup
    NULL,                 // valdup
    mdbStringObjKeyCompare, // keyCompare
    mdbDictMdbObjFree,    // keyFree
    mdbDictMdbObjFree     // valFree
};
mdbDb *mdbCreateDb(int dbnum) {
    mdbDb *db = (mdbDb *)mdbMalloc(sizeof(mdbDb) * dbnum);
    if(db == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateDb() | At %s:%d", __FILE__, __LINE__);
        return NULL;
    }
    for(int i = 0; i < dbnum; i++) {
        // 创建字典
        db[i].dict = mdbDictCreate(&gDBDtype);
        db[i].keynum = 0;
    }
    return db;
}

mdbClient *mdbCreateClient(int fd) {
    int ret = -1;
    mdbClient *client = (mdbClient *)mdbMalloc(sizeof(mdbClient));
    if(client == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbCreateClient() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    client->fd = fd;
    client->querybuf = NULL;
    client->replybuf = NULL;
    client->argc = 0;
    client->argv = NULL;
    // 默认选择第一个数据库
    client->db = &gServer.db[0];
    ret = 0;
__finish: 
    return ret == 0 ? client : NULL;
}

void mdbClientFree(void *c) {
    mdbClient *client = (mdbClient *)c;
    if(client == NULL) {
        return;
    }
    if(client->fd > 0) {
        close(client->fd);
    }
    if(client->querybuf != NULL) {
        mdbSdsfree(client->querybuf);
    }
    if(client->replybuf != NULL) {
        mdbSdsfree(client->replybuf);
    }
    for(int i = 0; i < client->argc; i++) {
        if(client->argv[i] != NULL) {
            mdbDecrRefCount(client->argv[i]);
        }
    }
    mdbFree(client);
}


int initMdbServer(void) {
    int ret = -1;
    gServer.clients = NULL;
    gServer.dbnum = 0;
    memset(gServer.ip, 0, sizeof(gServer.ip));
    gServer.port = 0;
    memset(gServer.logpath, 0, sizeof(gServer.logpath));
    gServer.loglevel = LOG_INFO;
    gServer.mdbCommands = NULL;
    gServer.db = NULL;
    // 初始化命令字典
    initCommandDict();
    // 加载配置
    loadConfig();
    if(gServer.dbnum <= 0) {
        mdbLogWrite(LOG_ERROR, "initMdbServer() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 初始化数据库
    gServer.db = mdbCreateDb(gServer.dbnum);
    if(gServer.db == NULL) {
        mdbLogWrite(LOG_ERROR, "initMdbServer() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 创建客户端链表
    gServer.clients = mdbListCreate(NULL, mdbClientFree, NULL);
    if(gServer.clients == NULL) {
        mdbLogWrite(LOG_ERROR, "initMdbServer() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }

    // 创建共享对象
    if(mdbCreateSharedObjects() < 0) {
        mdbLogWrite(LOG_ERROR, "initMdbServer() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

void destroyMdbServer(void) {
    if(gServer.db != NULL) {
        for(int i = 0; i < gServer.dbnum; i++) {
            mdbDictFree(gServer.db[i].dict);
        }
        mdbFree(gServer.db);
    }
    if(gServer.clients != NULL) {
        mdbListFree(gServer.clients);
    }
    if(gServer.mdbCommands != NULL) {
        mdbDictFree(gServer.mdbCommands);
    }
}

void loadBanner(void) {
    FILE *fp = fopen(BANNER_FILEPATH, "r");
    char buf[BUFFER_SIZE] = {0};
    if(fp == NULL) {
        mdbLogWrite(LOG_ERROR, "loadBanner() fopen() | At %s:%d", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    fread(buf, sizeof(char), BUFFER_SIZE, fp);
    fwrite(buf, sizeof(char), strlen(buf), stdout);
    fclose(fp);
    
}

/*
des
    后台运行
return
    成功: 0
    失败: -1
*/
int daemonize(void) {
    int ret = -1;
    pid_t pid = fork();
    if(pid < 0) {
        goto __finish;
    }
    if(pid > 0) {
        exit(EXIT_SUCCESS);
    }
    if(setsid() == -1) {
        goto __finish;
    }
    if(chdir("/") == -1) {
        goto __finish;
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    ret = 0;
__finish:
    return ret;
}

int mdbSendReply(int fd, char *reply , uint8_t code) {
    int ret = -1;
    uint16_t dataLen = strlen(reply);
    dataLen = htons(dataLen);
    if(mdbWrite(fd, &code, sizeof(code)) <= 0) {
        mdbLogWrite(LOG_ERROR, "mdbSendReply() | write() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(mdbWrite(fd, &dataLen, sizeof(dataLen)) <= 0) {
        mdbLogWrite(LOG_ERROR, "mdbSendReply() | write() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(mdbWrite(fd, reply, strlen(reply)) <= 0) {
        mdbLogWrite(LOG_ERROR, "mdbSendReply() | write() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

int mdbParseCmd(mdbClient *client, int fd) {
    int ret = -1;
    uint8_t buf[BIGBUFFER_SIZE] = {0};
    uint16_t dataLen = 0;
    char *token = NULL;
    int argc = 0;
    if(mdbRead(fd, &dataLen, sizeof(dataLen)) <= 0) {
        mdbLogWrite(LOG_ERROR, "mdbParseCmd() | read() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    dataLen = ntohs(dataLen);
    if(mdbRead(fd, buf, dataLen) <= 0) {
        mdbLogWrite(LOG_ERROR, "mdbParseCmd() | read() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    buf[dataLen] = '\0';
    client->querybuf = mdbSdsnew((char *)buf);
    if(client->querybuf == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbParseCmd() | mdbSdsnewlen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }

    
    for(int i = 0; i < dataLen; i++) {
        if(buf[i] == '\r' && buf[i + 1] == '\n') {
            argc++;
        }
    }
    client->argv = mdbMalloc(sizeof(mobj *) * argc);
    if(client->argv == NULL) {
        mdbLogWrite(LOG_ERROR, "mdbParseCmd() | mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    token = (char *)buf;
    client->argc = 0;
    for(int i = 0; i < dataLen; i++) {
        if(buf[i] == '\r' && buf[i + 1] == '\n') {
            buf[i] = '\0';
            buf[i + 1] = '\0';
            client->argv[client->argc++] = mdbCreateStringObject(token);
            token = (char *)buf + i + 2;
        }
    }
    // 通过命令寻找命令处理函数
    mdbCommand *cmd = mdbDictFetchValue(gServer.mdbCommands, client->argv[0]->ptr);
    if(cmd == NULL) {
        // 回复错误, 不知道的命令
        if(mdbSendReply(fd, "ERR: Unknown command\r\n", MDB_REP_ERROR) < 0) {
            mdbLogWrite(LOG_ERROR, "mdbParseCmd() | mdbSendReply() | At %s:%d", __FILE__, __LINE__);
            goto __finish;
        }

    } else {
        // 执行命令
        cmd->proc(client);
    }
    ret = 0;
__finish:
    for(int i = 0; i < client->argc; i++) {
        mdbDecrRefCount(client->argv[i]);
    }
    mdbFree(client->argv);
    mdbSdsfree(client->querybuf);
    mdbSdsfree(client->replybuf);
    client->argc = 0;
    client->argv = NULL;
    client->querybuf = NULL;
    client->replybuf = NULL;
    return ret;
}
/*
des:
    处理客户端读事件
param:

return:
    成功: 0
    失败: -1
*/
int clientFileEventProc(mdbEventLoop *eventLoop, int fd, void *clientData, int mask) {
    int ret = -1;
    mdbClient *client = (mdbClient *)clientData;
    if(client == NULL) {
        goto __finish;
    }
    // 对客户端进行初始化
    if(client->fd == -1) {
        client->fd = fd;
        client->querybuf = NULL;
        client->argc = 0;
        client->argv = NULL;
    }
    // 解析命令
    if(mdbParseCmd(client, fd) < 0) {
        mdbDeleteFileEvent(eventLoop, fd, MDB_READABLE);
        close(fd);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
    
}


/*
des:
    接收客户端描述符, 将接收到的客户端描述符放入事件循环
param:

return:
    成功: 0
    失败: -1
*/
int acceptFileEventProc(mdbEventLoop *eventLoop, int fd, void *clientData, int mask) {
    int ret = -1;
    struct sockaddr_in clientSock;
    memset(&clientSock, 0, sizeof(clientSock));
    socklen_t sockLen = 0;
    int clientFd = accept(fd, (struct sockaddr *)&clientSock, &sockLen);
    if(clientFd < 0) {
        mdbLogWrite(LOG_ERROR, "acceptFileEventProc() accept() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 打印客户端ip地址
    char* ip = inet_ntoa(clientSock.sin_addr);
    mdbLogWrite(LOG_INFO, "client ip: %s", ip);
    // 创建客户端
    mdbClient *client = mdbCreateClient(clientFd);
    // 将客户端放入客户端链表中
    if(mdbListAddNodeTail(gServer.clients, client) < 0) {
        mdbLogWrite(LOG_ERROR, "acceptFileEventProc() mdbListAddNodeTail() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    // 添加文件事件
    if(mdbCreateFileEvent(eventLoop, clientFd, MDB_READABLE, clientFileEventProc, client) < 0) {
        mdbLogWrite(LOG_ERROR, "acceptFileEventProc() mdbCreateFileEvent() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0; 
__finish:
    return ret;
}


/*
des
    开启服务
param
    ip: ip地址
    port: 端口
return
    成功: 0
    失败: -1    
*/
int startService(const char *ip, int port) {
    int ret = -1;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int clientfd = -1;
    if(listenfd == -1) {
        mdbLogWrite(LOG_ERROR, "startService() socket() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    struct sockaddr_in cliaddr;
    socklen_t cliaddrLen = 0;

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        mdbLogWrite(LOG_ERROR, "startService() bind() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(listen(listenfd, 1024) == -1) {
        mdbLogWrite(LOG_ERROR, "startService() listen() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    mdbLogWrite(LOG_INFO, "Server start at %s:%d", ip, port);

    mdbEventLoop *loop = mdbCreateEventLoop(1024);
    mdbLogWrite(LOG_DEBUG, "create EventLoop Completed!");
    if(loop == NULL) {
        mdbLogWrite(LOG_ERROR, "startService() mdbCreateEventLoop()  | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }

     // 添加文件事件
    if(mdbCreateFileEvent(loop, listenfd, MDB_READABLE, acceptFileEventProc, NULL) < 0) {
        mdbLogWrite(LOG_ERROR, "startService() mdbCreateFileEvent() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    if(mdbStartEventLoop(loop) < 0) {
        mdbLogWrite(LOG_ERROR, "startService() mdbStartEventLoop() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    ret = 0;
__finish:
    return ret;
}

int main(int argc, char **argv) {
    // 初始化mdbServer
    if(initMdbServer() < 0) {
        mdbLogWrite(LOG_ERROR, "main() initMdbServer() | At %s:%d", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    // 默认后台运行
    int daemon = 1;
    int opt;
    while ((opt = getopt(argc, argv, "i:p:d:hl:")) != -1) {
        switch (opt) {
            case 'i':
                memset(gServer.ip, 0, sizeof(gServer.ip));
                strncpy(gServer.ip, optarg, strlen(optarg));
                break;
            case 'p':
                gServer.port = atoi(optarg);
                break;
            case 'd':
                if(strcmp(optarg, "1") == 0) {
                    daemon = 1;
                } else {
                    daemon = 0;
                }
                break;
            case 'l':
                if(strcmp(optarg, "debug") == 0) {
                    gServer.loglevel = LOG_DEBUG;
                } else if(strcmp(optarg, "info") == 0) {
                    gServer.loglevel = LOG_INFO;
                } else if(strcmp(optarg, "warning") == 0) {
                    gServer.loglevel = LOG_WARNING;
                } else if(strcmp(optarg, "error") == 0) {
                    gServer.loglevel = LOG_ERROR;
                } else {
                    mdbLogWrite(LOG_ERROR, "Usage: %s [-i ip] [-p port] [-d]", argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                mdbLogWrite(LOG_INFO, "Usage: %s [-i ip] [-p port] [-d]", argv[0]);
                exit(EXIT_SUCCESS);
            default:
                mdbLogWrite(LOG_ERROR, "Usage: %s [-i ip] [-p port] [-d]", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if(gServer.port == 0) {
        gServer.port = DEFAULT_PORT;
    }
    if(strlen(gServer.ip) == 0) {
        strncpy(gServer.ip, DEFAULT_IP, strlen(DEFAULT_IP));
    }
    if(gServer.loglevel == 0) {
        gServer.loglevel = LOG_INFO;
    }
    if(strlen(gServer.logpath) == 0) {
        strncpy(gServer.logpath, LOG_PATH, strlen(LOG_PATH));
    }
    loadBanner();
    if(daemon) {
        mdbLogWrite(LOG_INFO, "Daemonizing...");
        if(daemonize() == -1) {
            mdbLogWrite(LOG_ERROR, "daemonize() failed | At %s:%d", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
        mdbLogInit(gServer.loglevel, gServer.logpath);
    }
    
    int listenfd = startService(gServer.ip, gServer.port);
    if(listenfd == -1) {
        mdbLogWrite(LOG_ERROR, "startService() failed | At %s:%d", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    return 0;
}