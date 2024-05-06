#include "mdb_common.h"
#include "mdb_config.h"
#include "mdb_sds.h"
FILE *configFile = NULL;
/*
des:
    Sds哈希函数
*/
unsigned int mdbConfigSdsHash(const void *v) {
    SDS *sds = (SDS *)v;
    return mdbBkdrHash(sds->buf);
}

int mdbConfigSdsKeyCompare(const void *key1, const void *key2) {
    int l1,l2;
    l1 = mdbSdslen((SDS *)key1);
    l2 = mdbSdslen((SDS *)key2);
    mdbLogWrite(LOG_DEBUG, "key1: %s, key2: %s", ((SDS *)key1)->buf, ((SDS *)key2)->buf);
    mdbLogWrite(LOG_DEBUG, "l1: %d, l2: %d", l1, l2);
    if (l1 != l2) return 1;
    return memcmp(((SDS *)key1)->buf, ((SDS *)key2)->buf, l1);
}

void mdbConfigSdsKeyFree(void *key) {
    mdbSdsfree((SDS *)key);
}

void mdbConfigSdsValFree(void *val) {
    mdbSdsfree((SDS *)val);
}
dictType gConfigDtype = {
    mdbConfigSdsHash,           // hash function
    NULL,                   // keydup
    NULL,                   // valdup
    mdbConfigSdsKeyCompare, // keyCompare
    mdbConfigSdsKeyFree,             // keyFree
    mdbConfigSdsValFree              // valFree
};
/*
des:
    加载配置文件
params:
    configFilepath: 配置文件路径
return:
    成功: 0
    失败: -1
*/
int mdbLoadConfig(char *configFilepath) {
    configFile = fopen(configFilepath, "r");
    if (configFile == NULL) {
        return -1;
    }
    return 0;
} 
/*
des:
    获取配置
return:
    成功: 配置字典
    失败: NULL
*/
dict *mdbGetConfig() {
    dict *configDict = mdbDictCreate(&gConfigDtype);
    if (configDict == NULL) {
        return NULL;
    }
    char line[1024];
    while (fgets(line, 1024, configFile) != NULL) {
        int flag = 0;
        for(int i = 0; i < strlen(line); i++) {
            if(line[i] == ' ') {
                continue;
            }
            if(line[i] == '#') {
                flag = 1;
                break;
            }
        }
        if(flag == 1) {
            continue;
        }
        SDS *key = mdbSdsnew(strtok(line, "=\n\r"));
        
        SDS *value = mdbSdsnew(strtok(NULL, "=\n\r"));
        if (mdbSdslen(key) == 0 || mdbSdslen(value) == 0) {
            continue;
        }
        mdbLogWrite(LOG_DEBUG, "key: %s, value: %s", key->buf, value->buf);
        mdbDictAdd(configDict, key, value);
    }
    return configDict;
}