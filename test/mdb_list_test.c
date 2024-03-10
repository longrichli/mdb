#include "../src/mdb_common.h"
#include "../src/mdb_list.h"
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

void strFree(void *str) {
    mdbFree(str);
}

int strMatch(void *str1, void *str2) {
    if(str1 == NULL && str2 == NULL) {
        return 1;
    } else if(str1 == NULL || str2 == NULL) {
        return 0;
    } else if(strcmp((char *)str1, (char *)str2) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void listPrint(linkedList *list) {
    listNode *pNode = list->head;
    while(pNode != NULL) {
        mdbLogWrite(LOG_INFO, "val: %s", (char *)pNode->value);
        pNode = pNode->next;
    }
}

int main(void) {
    linkedList *list = mdbListCraete(strDup, strFree, strMatch);
    listNode *tmpNode = NULL;
    char *str1 = strDup("1");
    char *str2 = strDup("2");
    char *str3 = strDup("3");
    char *str4 = strDup("4");
    char *str5 = strDup("5");
    char *str6 = strDup("6");
    char *str7 = strDup("7");
    mdbAddNodeTail(list, str1);
    mdbAddNodeTail(list, str2);
    mdbAddNodeTail(list, str3);
    mdbAddNodeTail(list, str4);
    mdbAddNodeHead(list, str5);
    tmpNode = mdbListSearchKey(list, "3");
    mdbListInsertNode(list, tmpNode, str6, 1);
    tmpNode = mdbListIndex(list, 4);
    mdbListInsertNode(list, tmpNode, str7, 0);
    mdbLogWrite(LOG_INFO, "list->len = %lu", list->len);
    listPrint(list);
    mdbLogWrite(LOG_INFO, "============================");
    mdbListDelNode(list, tmpNode);
    listPrint(list);
    linkedList *newList = mdbListDup(list);
    mdbLogWrite(LOG_INFO, "============================");
    listPrint(newList);
    mdbListFree(list);
    mdbListFree(newList);
    return 0;
}