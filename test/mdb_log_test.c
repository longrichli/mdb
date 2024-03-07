#include "../src/mdb_common.h"

int main(int argc, char **argv) {
    mdbLogInit(LOG_INFO, "test.log");
    mdbLogWrite(LOG_INFO, "START TEST");
    int a = rand();
    mdbLogWrite(LOG_DEBUG, "a = %d", a);
    if(a % 2 != 0) {
        mdbLogWrite(LOG_WARNING, "It is base number.");
        mdbLogWrite(LOG_ERROR, "EXIT!");
        exit(EXIT_FAILURE);
    }
    return 0;
}