#include <stdio.h>
#include "../include/mdb_cli_lib.h"
#include <malloc.h>
#include <string.h>
#include <unistd.h>
typedef struct cmd {
    char *cmd;
    char *desc;
} cmd;
cmd cmds[] = {
    {"set str1 abc", "将字符串abc添加到数据库中"},
    {"set str2 def", "将字符串def添加到数据库中"},
    {"set num1 123", "将数字123添加到数据库中"},
    {"set num2 456", "将数字456添加到数据库中"},
    {"get str1", "获取str1表示的字符串"},
    {"get str2", "获取str2表示的字符串"},
    {"get num1", "获取num1表示的数字"},
    {"get num2", "获取num2表示的数字"},
    {"append str1 def", "将字符串def追加到str1表示的字符串后面"},
    {"get str1", "获取str1表示的字符串"},
    {"append str2 abc", "将字符串abc追加到str2表示的字符串后面"},
    {"get str2", "获取str2表示的字符串"},
    {"append num1 789", "将数字789追加到num1表示的数字后面"},
    {"get num1", "获取num1表示的数字"},
    {"append num2 123", "将数字123追加到num2表示的数字后面"},
    {"get num2", "获取num2表示的数字"},
    {"incrby num1 10", "将num1表示的数字加10"},
    {"get num1", "获取num1表示的数字"},
    {"incrby num2 20", "将num2表示的数字加20"},
    {"get num2", "获取num2表示的数字"},
    {"decrby num1 5", "将num1表示的数字减5"},
    {"get num1", "获取num1表示的数字"},
    {"decrby num2 10", "将num2表示的数字减10"},
    {"get num2", "获取num2表示的数字"},
    {"incr num1", "将num1表示的数字加1"},
    {"get num1", "获取num1表示的数字"},
    {"strlen str1", "获取str1表示的字符串的长度"},
    {"strlen str2", "获取str2表示的字符串的长度"},
    {"strlen num1", "获取num1表示的数字的长度"},
    {"strlen num2", "获取num2表示的数字的长度"},
    {"setrange str1 3 123", "将字符串123替换到str1表示的字符串的第3个字符之后"},
    {"get str1", "获取str1表示的字符串"},
    {"setrange str2 0 456", "将字符串456替换到str2表示的字符串的第0个字符之后"},
    {"get str2", "获取str2表示的字符串"},
    {"getrange str1 0 2", "获取str1表示的字符串的第0个字符到第2个字符的子串"},
    {"getrange str2 3 5", "获取str2表示的字符串的第3个字符到第5个字符的子串"},

    // 列表相关命令
    {"lpush list a b c", "将元素a,b,c依次添加到列表list的表头"},
    {"rpush list x y z", "将元素x,y,z依次添加到列表list的表尾"},
    {"lrange list 0 -1", "获取列表list的全部元素"},
    {"llen list", "获取列表list的长度"},
    {"lindex list 1", "获取列表list的索引为1的元素"},
    {"lset list 1 123", "将列表list的索引为1的元素设置为123"},
    {"lrange list 0 -1", "获取列表list的全部元素"},
    {"lpop list", "从列表list的表头弹出一个元素"},
    {"lrange list 0 -1", "获取列表list的全部元素"},
    {"rpop list", "从列表list的表尾弹出一个元素"},
    {"lrange list 0 -1", "获取列表list的全部元素"},
    {"ltrim list 1 2", "截取列表list的索引为1到2的元素"},
    {"lrange list 0 -1", "获取列表list的全部元素"},

    // 哈希表相关命令
    {"hset hash a 1 b 2 c 3", "将键值对a:1,b:2,c:3添加到哈希表hash中"},
    {"hget hash a", "获取哈希表hash中键为a的值"},
    {"hget hash b", "获取哈希表hash中键为b的值"},
    {"hget hash c", "获取哈希表hash中键为c的值"},
    {"hgetall hash", "获取哈希表hash中的所有键值对"},
    {"hdel hash a", "删除哈希表hash中键为a的键值对"},
    {"hgetall hash", "获取哈希表hash中的所有键值对"},
    {"hset hash b 123", "将哈希表hash中键为b的值设置为123"},
    {"hget hash b", "获取哈希表hash中键为b的值"},
    {"hgetall hash", "获取哈希表hash中的所有键值对"},
    {"hkeys hash", "获取哈希表hash中的所有键"},
    {"hlen hash", "获取哈希表hash的长度"},
    {"hexists hash b", "判断哈希表hash中是否存在键为b的键值对"},
    {"hexists hash z", "判断哈希表hash中是否存在键为z的键值对"},

    // 集合相关命令
    {"sadd set a b c d", "将元素a,b,c,d依次添加到集合set中"},
    {"scard set", "获取集合set的元素个数"},
    {"smembers set", "获取集合set中的所有元素"},
    {"sismember set a", "判断集合set中是否存在元素a"},
    {"sismember set z", "判断集合set中是否存在元素z"},
    {"srem set a", "删除集合set中元素a"},
    {"smembers set", "获取集合set中的所有元素"},
    {"srandmember set", "随机获取集合set中的一个元素"},
    {"srandmember set", "随机获取集合set中的一个元素"},
    {"srandmember set", "随机获取集合set中的一个元素"},
    {"spop set 1", "随机弹出集合set中的一个元素"},
    {"smembers set", "获取集合set中的所有元素"},
    // 有序集合相关命令
    {"zadd zset 1.2 apple 3.4 banana 2.4 cherry", "将元素apple,banana,cherry,分别加上分数1.2,3.4,2.4,添加到有序集合zset中"},
    {"zcard zset", "获取有序集合set的元素个数"},
    {"zrange zset 0 -1 withscores", "获取有序集合zset中的全部元素及其分数"},
    {"zrevrange zset 0 -1 withscores", "获取有序集合zset中的全部元素及其分数,按分数逆序"},
    {"zrank zset apple", "获取元素apple在有序集合zset中的排名"},
    {"zrevrank zset apple", "获取元素apple在有序集合zset中的反向排名"},
    {"zscore zset apple", "获取元素apple在有序集合zset中的分数"},
    {"zcount zset 1.2 3.4", "获取有序集合zset中分数在1.2到3.4之间的元素个数"},
    {"zrem zset apple", "删除有序集合zset中元素apple"},
    {"zrange zset 0 -1 withscores", "获取有序集合zset中的全部元素及其分数"},

    // 通用命令
    {"keys *", "获取当前数据库中的所有键"},
    {"type str1", "获取键str1的类型"},
    {"type list", "获取键list的类型"},
    {"del str1", "删除键str1"},
    {"keys *", "获取当前数据库中的所有键"},
    {"select 1", "切换到数据库1"},
    {"set str1 123", "向数据库中添加字符串123"},
    {"get str1", "向数据库中获取字符串"},
    {"select 0", "切换到数据库0"},
    {"keys *", "获取当前数据库中的所有键"},
    {"select 1", "切换到数据库1"},
    {"keys *", "获取当前数据库中的所有键"}
};

int main(int argc, char **argv) {
    char *result = NULL;
    uint8_t code = 0;
    int serverFd = connectToServer("127.0.0.1", 8181);
    for(int i = 0; i < sizeof(cmds)/sizeof(cmd); i++) {
        printf("Executing command: %s\n", cmds[i].cmd);
        printf("Description: %s\n", cmds[i].desc);
        sendCommand(serverFd, cmds[i].cmd);
        readResault(serverFd, &result, &code);
        printf("%s\n", result);
        free(result);
    }
    return 0;
}