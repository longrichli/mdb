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
    for(int i = 0; i < 1000; i++) {
        char keybuf[10] = {0};
        char valbuf[10] = {0};
        sprintf(keybuf, "key%d", i);
        sprintf(valbuf, "val%d", i);
        char *key = strDup(keybuf);
        char *val = strDup(valbuf);
        mdbDictAdd(d, key, val);
    }
    // for(int i = 0; i < 1000; i++) {
    //     char keybuf[10] = {0};
    //     sprintf(keybuf, "key%d", i);
    //     char *val = mdbDictFetchValue(d, keybuf);
    //     printf("%s\n", val);
    // }
    for(int i = 0; i < 920; i++) {
        char keybuf[10] = {0};
        sprintf(keybuf, "key%d", i);
        mdbDictDelete(d, keybuf);
    }

    for(int i = 0; i < 1000; i++) {
        char keybuf[10] = {0};
        sprintf(keybuf, "key%d", i);
        char *val = mdbDictFetchValue(d, keybuf);
        printf("%s\n", val == NULL ? "null" : val);
    }
    mdbDictFree(d);
    return 0;
}