#include "../src/mdb_common.h"
#include "../src/mdb_dict.h"
#include "../src/mdb_alloc.h"

void *strDup(void *str) {
    int ret = -1;
    char *dupString = NULL;
    if(str == NULL) {
        mdbLogWrite(LOG_ERROR, "dupStr() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    dupString = mdbMalloc(strlen((char *)str) + 1);
    if(dupString == NULL) {
        mdbLogWrite(LOG_ERROR, "dupStr() mdbMalloc() | At %s:%d", __FILE__, __LINE__);
        goto __finish;
    }
    strcpy(dupString, (char *)str);
    ret = 0;
__finish:
    return ret == 0 ? dupString : NULL;
}

// BKDR哈希函数
unsigned int bkdrHash(const void *key) {
    unsigned int seed = 131; // 也可以选择31、131、1313、13131、131313等
    unsigned int hash = 0;
    char *str = (char *)key;
    while (*str) {
        hash = hash * seed + (*str++);
    }

    return hash;
}

int strCompare(const void *str1, const void *str2) {
    if(str1 == NULL && str2 == NULL) {
        return 0;
    } else if(str1 ==  NULL || str2 == NULL) {
        return 1;
    } else {
        return strcmp((char *)str1, (char *)str2);
    }
}

int main(void) {
    dictType type = {
        bkdrHash,
        NULL,
        NULL,
        strCompare,
        mdbFree,
        mdbFree
    };
    dict *d = mdbDictCreate(&type);
    if(d == NULL) {
        mdbLogWrite(LOG_ERROR, "main() mdbDictCreate()");
        exit(EXIT_FAILURE);
    }
    char *key1 = strDup("11");
    char *key2 = strDup("21");
    char *key3 = strDup("31");
    char *key4 = strDup("4");
    char *key5 = strDup("5");
    char *key6 = strDup("6");
    char *key7 = strDup("6");

    char *val1 = strDup("one");
    char *val2 = strDup("two");
    char *val3 = strDup("three");
    char *val4 = strDup("four");
    char *val5 = strDup("five");
    char *val6 = strDup("six");
    char *val7 = strDup("seven");

    mdbDictAdd(d, key1, val1);
    mdbDictAdd(d, key2, val2);
    mdbDictAdd(d, key3, val3);
    mdbDictAdd(d, key4, val4);
    mdbDictAdd(d, key5, val5);
    mdbDictAdd(d, key6, val6);
    mdbDictReplace(d, key7, val7);
    void *v1 = mdbDictFetchValue(d, key1);
    printf("%s\n", (char *)v1);
    // void *v2 = mdbDictFetchValue(d, key2);
    // printf("%s\n", (char *)v2);
    // void *v3 = mdbDictFetchValue(d, key3);
    // printf("%s\n", (char *)v3);
    // void *v4 = mdbDictFetchValue(d, key4);
    // printf("%s\n", (char *)v4);
    // void *v5 = mdbDictFetchValue(d, key5);
    // printf("%s\n", (char *)v5);
    // void *v6 = mdbDictFetchValue(d, "6");
    // printf("%s\n", (char *)v6);
    return 0;
}