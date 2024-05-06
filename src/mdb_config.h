#ifndef __MDB_CONFIG_H__
#define __MDB_CONFIG_H__
#include "mdb_dict.h"
/*
des:
    加载配置文件
params:
    configFilepath: 配置文件路径
return:
    成功: 0
    失败: -1
*/
int mdbLoadConfig(char *configFilepath);
/*
des:
    获取配置
return:
    成功: 配置字典
    失败: NULL
*/
dict *mdbGetConfig();
#endif /* __MDB_CONFIG_H__ */