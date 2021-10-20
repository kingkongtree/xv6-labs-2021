#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(2, "pingpong with invalid argc %d\n", argc);
        exit(1);
    }

    int rx[2], tx[2];
    char ball[5];

    pipe(rx);
    pipe(tx);

    if (fork() == 0) {
        close(tx[1]);
        close(rx[0]);
        if (read(tx[0], ball, 5) == 5) {
            fprintf(0, "%d: received %s\n", getpid(), ball);
            write(rx[1], "pong", 5);
            exit(0);
        }
        exit(1);
    }

    close(rx[1]);
    close(tx[0]);
    write(tx[1], "ping", 5);
    if (read(rx[0], ball, 5) == 5) {
        fprintf(0, "%d: received %s\n", getpid(), ball);
        exit(0);
    }
    exit(1);
}