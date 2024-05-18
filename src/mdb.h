#ifndef __MDB_H__
#define __MDB_H__
#include "mdb_list.h"
#include "mdb_dict.h"
#include "mdb_sds.h"
#include "mdb_object.h"
#include "mdb_common.h"

#define MDB_REP_OK (0)
#define MDB_REP_ERROR (-1)
#define MDB_REP_STRING (1)
#define MDB_REP_ARRAY (2)
#define MDB_REP_NIL (3)

typedef struct mdbDb {
    dict *dict;
    size_t keynum;
} mdbDb;



typedef struct mdbServer {
    int port;
    char ip[16];
    char logpath[BUFFER_SIZE];
    int loglevel;
    mdbDb *db;
    int dbnum;
    linkedList *clients;
    dict *mdbCommands;
} mdbServer;

typedef struct mdbClient {
    int fd;
    mobj *name;
    mdbDb *db;
    int argc;
    mobj **argv;
    SDS *querybuf;
    SDS *replybuf;
} mdbClient;

typedef void cmdProc(mdbClient *c);
typedef struct mdbCommand {
    SDS *name;
    cmdProc *proc;
} mdbCommand;

int mdbSendReply(int fd, char *reply , uint8_t code);

// 通用命令

// SELECT：切换到指定的数据库。
// 例如：SELECT db_index
void mdbCommandSelect(mdbClient *c);

// KEYS：用于查找满足指定模式的键。
// 例如：KEYS *、KEYS user:*
void mdbCommandKeys(mdbClient *c);
// DEL：用于删除一个或多个键。
// 例如：DEL key1 key2
void mdbCommandDel(mdbClient *c);
// EXISTS：用于检查指定键是否存在。
// 例如：EXISTS key
void mdbCommandExists(mdbClient *c);
// TYPE：用于获取指定键的数据类型。
// 例如：TYPE key
void mdbCommandType(mdbClient *c);
// RENAME：用于重命名一个键。
// 例如：RENAME old_key new_key
void mdbCommandRename(mdbClient *c);
// RENAMEX：用于重命名一个键，仅在新键不存在时执行。
// 例如：RENAMEX old_key new_key
void mdbCommandRenamex(mdbClient *c);
// EXPIRE：设置键的过期时间（以秒为单位）。
// 例如：EXPIRE key seconds
void mdbCommandExpire(mdbClient *c);
// TTL：获取键的剩余过期时间（以秒为单位）。
// 例如：TTL key
void mdbCommandTtl(mdbClient *c);
// PERSIST：移除键的过期时间，使其永不过期。
// 例如：PERSIST key
void mdbCommandPersist(mdbClient *c);
// SCAN：迭代数据库中的键。
// 例如：SCAN cursor [MATCH pattern] [COUNT count]
void mdbCommandScan(mdbClient *c);

// 字符串相关命令
// SET	向内存数据库中存入一个字符串。
// 例如：SET key value
void mdbCommandSet(mdbClient *c);
// GET	从内存数据库中取出一个字符串。
// 例如：GET key
void mdbCommandGet(mdbClient *c);
// APPEND	如果内存数据库存在某个字符串，对这个字符串进行追加，如果内存数据库不存在指定的字符串，则新存入一个字符串，字符串的内容为追加的内容。
// 例如：APPEND key value
void mdbCommandAppend(mdbClient *c);
// INCRBY	如果字符串可以转换成整数，对其进行加法运算，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：INCRBY key increment
void mdbCommandIncrby(mdbClient *c);
// DECRBY	如果字符串可以转换成整数，对其进行减法运算，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：DECRBY key decrement
void mdbCommandDecrby(mdbClient *c);
// INCR	如果字符串可以转换成整数，对其进行加1，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：INCR key
void mdbCommandIncr(mdbClient *c);
// STRLEN	返回指定字符串的长度。
// 例如：STRLEN key
void mdbCommandStrlen(mdbClient *c);
// SETRANGE	指定一个偏移量，覆盖指定字符串偏移量后面的部分。
// 例如：SETRANGE key offset value
void mdbCommandSetrange(mdbClient *c);
// GETRANGE	指定一个范围，返回字符串落在该范围内的字串。
// 例如：GETRANGE key start end
void mdbCommandGetrange(mdbClient *c);

// 列表相关命令
// LPUSH	添加元素到列表的表头。
// 例如：LPUSH key value1 value2
void mdbCommandLpush(mdbClient *c);
// RPUSH	添加元素到列表的表尾。
// 例如：RPUSH key value1 value2
void mdbCommandRpush(mdbClient *c);
// LPOP	弹出列表的表头节点。
// 例如：LPOP key
void mdbCommandLpop(mdbClient *c);
// RPOP	弹出列表的表尾节点。
// 例如：RPOP key
void mdbCommandRpop(mdbClient *c);
// LINDEX	返回列表中指定索引的节点。
// 例如：LINDEX key index
void mdbCommandLindex(mdbClient *c);
// LLEN	返回列表的长度。
// 例如：LLEN key
void mdbCommandLlen(mdbClient *c);
// LINSERT	在列表中指定元素的前面或后面添加新元素，如果列表中不存在指定元素，列表保持不变。
// 例如：LINSERT key BEFORE pivot value
void mdbCommandLinsert(mdbClient *c);
// LREM	删除列表中指定的节点。
// 例如：LREM key count value
void mdbCommandLrem(mdbClient *c);
// LTRIM	修剪列表在指定的范围内。
// 例如：LTRIM key start stop
void mdbCommandLtrim(mdbClient *c);
// LSET	设置列表中指定索引的元素。
// 例如：LSET key index value
void mdbCommandLset(mdbClient *c);
// LRANGE	返回指定范围的列表元素。
// 例如：LRANGE key start stop
void mdbCommandLrange(mdbClient *c);

// 哈希相关命令
// HSET	向内存数据库中添加键值对。
// 例如：HSET key field value
void mdbCommandHset(mdbClient *c);
// HGET	输入键获取值。
// 例如：HGET key field
void mdbCommandHget(mdbClient *c);
// HEXSITS	查看一个键是否存在。
// 例如：HEXSITS key field
void mdbCommandHexists(mdbClient *c);
// HDEL	根据键删除该键所在的键值对。
// 例如：HDEL key field
void mdbCommandHdel(mdbClient *c);
// HLEN	返回哈希表中键值对的数量。
// 例如：HLEN key
void mdbCommandHlen(mdbClient *c);
// HKEYS	返回哈希表中的所有键。
// 例如：HKEYS key
void mdbCommandHkeys(mdbClient *c);
// HGETALL	返回哈希表中所有的键值对。
// 例如：HGETALL key
void mdbCommandHgetall(mdbClient *c);

// 集合相关命令
// SADD	向集合内添加元素。
// 例如：SADD key member1 member2
void mdbCommandSadd(mdbClient *c);
// SCARD	返回集合内元素的数量。
// 例如：SCARD key
void mdbCommandScard(mdbClient *c);
// SISMEMBER	查看指定元素是否存在于集合中。
// 例如：SISMEMBER key member
void mdbCommandSismember(mdbClient *c);
// SMEMBERS	返回集合内所有的元素。
// 例如：SMEMBERS key
void mdbCommandSmembers(mdbClient *c);
// SRANDMEMBER	随机返回集合中的一个元素。
// 例如：SRANDMEMBER key
void mdbCommandSrandmember(mdbClient *c);
// SPOP	随机返回并移除集合中一个或多个元素。
// 例如：SPOP key count
void mdbCommandSpop(mdbClient *c);
// SREM	移除集合中指定的元素。
// 例如：SREM key member1 member2
void mdbCommandSrem(mdbClient *c);


// 有序集合相关命令
// ZADD	将元素和分值添加到有序集合中。
// 例如：ZADD key score1 member1 score2 member2
void mdbCommandZadd(mdbClient *c);
// ZCARD	返回有序集合中元素的数量。
// 例如：ZCARD key
void mdbCommandZcard(mdbClient *c);
// ZCOUNT	返回分值在指定范围内的元素数量。
// 例如：ZCOUNT key min max
void mdbCommandZcount(mdbClient *c);
// ZRANGE	返回索引范围内的元素。
// 例如：ZRANGE key start stop
void mdbCommandZrange(mdbClient *c);
// ZREVRANGE	按分值从大到小返回索引范围内的元素。
// 例如：ZREVRANGE key start stop
void mdbCommandZrevrange(mdbClient *c);
// ZRANK	返回元素的排名。
// 例如：ZRANK key member
void mdbCommandZrank(mdbClient *c);
// ZREVRANK	返回元素的反向排名。
// 例如：ZREVRANK key member
void mdbCommandZrevrank(mdbClient *c);
// ZREM	删除给定的元素及其分值。
// 例如：ZREM key member1 member2
void mdbCommandZrem(mdbClient *c);
// ZSCORE	返回指定元素的分值。
// 例如：ZSCORE key member
void mdbCommandZscore(mdbClient *c);









#endif /* __MDB_H__ */