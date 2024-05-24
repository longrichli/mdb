#include <stdio.h>
#include <string.h>
/*
des: 
    字符串匹配
param:
    s: 字符串
    p: 模式串
return:
    匹配: 1
    不匹配: 0
*/
int mdbStrMatch(const char* reg, const char *str) {
    int r_len = (int)strlen(reg);
    int r_p = 0;
    int r_p_last = -1;
    int s_len = (int)strlen(str);
    int s_p = 0;
    int s_p_last = -1;
 
    while (s_p < s_len) {
        if (r_p < r_len && (*(str + s_p) == *(reg + r_p))) {
            r_p++;
            s_p++;
        }
        else if (r_p < r_len && (*(reg + r_p) == '*')) {
            r_p_last = r_p;
            r_p++;
            s_p_last = s_p;
        }
        else if (r_p_last > -1) {
            r_p = r_p_last + 1;
            s_p_last++;
            s_p = s_p_last;
        }
        else {
            return 0;//false
        }
    }
    while (r_p < r_len && (*(reg + r_p) == '*')) {
        r_p++;
    }
    if (r_p == r_len) {
        return 1;//true
    }
    return 0;//false
}

int main(int argc, char **argv) {

    printf("%d\n", mdbStrMatch("123*12", "12343122"));
    return 0;
}