#include "mdb.h"

// 列表相关命令
// LPUSH	添加元素到列表的表头。
// 例如：LPUSH key value1 value2
void mdbCommandLpush(mdbClient *c) {

}
// RPUSH	添加元素到列表的表尾。
// 例如：RPUSH key value1 value2
void mdbCommandRpush(mdbClient *c) {

}
// LPOP	弹出列表的表头节点。
// 例如：LPOP key
void mdbCommandLpop(mdbClient *c) {

}
// RPOP	弹出列表的表尾节点。
// 例如：RPOP key
void mdbCommandRpop(mdbClient *c) {

}
// LINDEX	返回列表中指定索引的节点。
// 例如：LINDEX key index
void mdbCommandLindex(mdbClient *c) {

}
// LLEN	返回列表的长度。
// 例如：LLEN key
void mdbCommandLlen(mdbClient *c) {

}
// LINSERT	在列表中指定元素的前面或后面添加新元素，如果列表中不存在指定元素，列表保持不变。
// 例如：LINSERT key BEFORE pivot value
void mdbCommandLinsert(mdbClient *c) {

}
// LREM	删除列表中指定的节点。
// 例如：LREM key count value
void mdbCommandLrem(mdbClient *c) {

}
// LTRIM	修剪列表在指定的范围内。
// 例如：LTRIM key start stop
void mdbCommandLtrim(mdbClient *c) {

}
// LSET	设置列表中指定索引的元素。
// 例如：LSET key index value
void mdbCommandLset(mdbClient *c) {

}
// LRANGE	返回指定范围的列表元素。
// 例如：LRANGE key start stop
void mdbCommandLrange(mdbClient *c) {

}