#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(2, "uptime with invalid argc %d\n", argc);
        exit(1);
    }

    int ret = uptime();
    printf("%d\n", ret);
    exit(0);
}