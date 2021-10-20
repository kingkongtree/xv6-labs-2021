#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(2, "sleep with invalid argc %d\n", argc);
        exit(1);
    }

    int tick = atoi(argv[1]);
    if (tick == 0) {
        fprintf(2, "sleep with invalid tick %s\n", argv[1]);
        exit(1);
    }

    sleep(tick);
    exit(0);
}