#include "mdb.h"

// 有序集合相关命令
// ZADD	将元素和分值添加到有序集合中。
// 例如：ZADD key score1 member1 score2 member2
void mdbCommandZadd(mdbClient *c) {

}
// ZCARD	返回有序集合中元素的数量。
// 例如：ZCARD key
void mdbCommandZcard(mdbClient *c) {

}
// ZCOUNT	返回分值在指定范围内的元素数量。
// 例如：ZCOUNT key min max
void mdbCommandZcount(mdbClient *c) {

}
// ZRANGE	返回索引范围内的元素。
// 例如：ZRANGE key start stop
void mdbCommandZrange(mdbClient *c) {

}
// ZREVRANGE	按分值从大到小返回索引范围内的元素。
// 例如：ZREVRANGE key start stop
void mdbCommandZrevrange(mdbClient *c) {

}
// ZRANK	返回元素的排名。
// 例如：ZRANK key member
void mdbCommandZrank(mdbClient *c) {

}
// ZREVRANK	返回元素的反向排名。
// 例如：ZREVRANK key member
void mdbCommandZrevrank(mdbClient *c) {

}
// ZREM	删除给定的元素及其分值。
// 例如：ZREM key member1 member2
void mdbCommandZrem(mdbClient *c) {

}
// ZSCORE	返回指定元素的分值。
// 例如：ZSCORE key member
void mdbCommandZscore(mdbClient *c) {

}