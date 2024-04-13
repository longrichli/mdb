#include "mdb_common.h"

int main(int argc, char **argv) {
    printf("[mdb 127.0.0.1:9999(0)]# \n");
    printf("[mdb 127.0.0.1:9999(0)]# \n");
    printf("[mdb 127.0.0.1:9999(0)]# \n");
    printf("[mdb 127.0.0.1:9999(0)]# \n");
    printf("[mdb 127.0.0.1:9999(0)]# set str str1\n");
    printf("OK\n");
    printf("[mdb 127.0.0.1:9999(0)]# get str\n");
    printf("\"str1\"\n");
    printf("[mdb 127.0.0.1:9999(0)]# lpush list a b c d\n");
    printf("(integer) 4\n");
    printf("[mdb 127.0.0.1:9999(0)]# lrange list 0 -1\n");
    printf("\"d\"\n");
    printf("\"c\"\n");
    printf("\"b\"\n");
    printf("\"a\"\n");
    printf("[mdb 127.0.0.1:9999(0)]# select 1\n");
    printf("OK\n");
    printf("[mdb 127.0.0.1:9999(1)]# ");
    getchar();
    return 0;
}