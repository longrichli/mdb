#include "mdb.h"

// 字符串相关命令
// SET	向内存数据库中存入一个字符串。
// 例如：SET key value
void mdbCommandSet(mdbClient *c) {

}
// GET	从内存数据库中取出一个字符串。
// 例如：GET key
void mdbCommandGet(mdbClient *c) {

}
// APPEND	如果内存数据库存在某个字符串，对这个字符串进行追加，如果内存数据库不存在指定的字符串，则新存入一个字符串，字符串的内容为追加的内容。
// 例如：APPEND key value
void mdbCommandAppend(mdbClient *c) {

}
// INCRBY	如果字符串可以转换成整数，对其进行加法运算，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：INCRBY key increment
void mdbCommandIncrby(mdbClient *c) {

}
// DECRBY	如果字符串可以转换成整数，对其进行减法运算，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：DECRBY key decrement
void mdbCommandDecrby(mdbClient *c) {

}
// INCR	如果字符串可以转换成整数，对其进行加1，将运算结果保存。如果不能转成整数，返回一个错误。
// 例如：INCR key
void mdbCommandIncr(mdbClient *c) {

}
// STRLEN	返回指定字符串的长度。
// 例如：STRLEN key
void mdbCommandStrlen(mdbClient *c) {

}
// SETRANGE	指定一个偏移量，覆盖指定字符串偏移量后面的部分。
// 例如：SETRANGE key offset value
void mdbCommandSetrange(mdbClient *c) {

}
// GETRANGE	指定一个范围，返回字符串落在该范围内的字串。
// 例如：GETRANGE key start end
void mdbCommandGetrange(mdbClient *c) {

}