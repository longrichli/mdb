#include <time.h>
#include <stdarg.h>
#include "mdb_common.h"


// 定义 ANSI 转义序列来设置文本颜色
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define DEFAULT_LEVEL LOG_DEBUG
static char gLogFilename[FILENAME_MAX] = {'\0'};
static logLevel gLevel = 0;
void mdbLogInit(logLevel level, char *filename) {
    gLevel = level;
    if(filename != NULL) {
        strcpy(gLogFilename, filename);
    } 
}

static void getcurrtime(char *timebuf) {
    time_t currentTime = 0;
    struct tm *localTime = NULL;
    /* 获取当前时间的时间戳 */
    currentTime = time(NULL);
    /* 将时间戳转换为本地时间 */
    localTime = localtime(&currentTime);

    sprintf(timebuf, "%d-%02d-%02d %02d:%02d:%02d",
           localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
           localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
}
void mdbLogWrite(logLevel level, char *fmt, ...) {
    FILE * fp = NULL;
    int stdOut = 0;
    char currtime[20];
    va_list args;
    if(level >= gLevel) {
        if(strlen(gLogFilename) == 0) {
            fp = stdout;
            stdOut = 1;
        } else {
            fp = mdbCreateFile(gLogFilename, "a+");
            if(fp == NULL) {
                abort();
            }
        }
        switch(level) {
            case LOG_DEBUG:
                fwrite("[DEBUG  ]", sizeof(char), 9, fp);
                break;
            case LOG_INFO:
                if(stdOut == 1) fwrite(ANSI_COLOR_GREEN, sizeof(char), 5, fp);
                fwrite("[INFO   ]", sizeof(char), 9, fp);
                if(stdOut == 1) fwrite(ANSI_COLOR_RESET, sizeof(char), 4, fp);
                break;
            case LOG_WARNING:
                if(stdOut == 1) fwrite(ANSI_COLOR_YELLOW, sizeof(char), 5, fp);
                fwrite("[WARNING]", sizeof(char), 9, fp);
                if(stdOut == 1) fwrite(ANSI_COLOR_RESET, sizeof(char), 4, fp);
                break;
            case LOG_ERROR:
                if(stdOut == 1) fwrite(ANSI_COLOR_RED, sizeof(char), 5, fp);
                fwrite("[ERROR  ]", sizeof(char), 9, fp);
                if(stdOut == 1) fwrite(ANSI_COLOR_RESET, sizeof(char), 4, fp);
                break; 
        }
        fwrite("[", sizeof(char), 1, fp);
        // 获取当前时间
        getcurrtime(currtime);
        fwrite(currtime, sizeof(char), 19, fp);
        fwrite("]: ", sizeof(char), 3, fp);
        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        va_end(args);
        fwrite("\n", sizeof(char), 1, fp);
        if(stdOut != 1) {
            fclose(fp);
        }
    }

    
}