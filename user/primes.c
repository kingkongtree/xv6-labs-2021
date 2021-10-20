#include "kernel/types.h"
#include "user/user.h"

void proc(int *pp)
{
    int first;

    close(pp[1]); // close pre write

    if (read(pp[0], &first, 4)) { // cur recv first
        // printf("@%d proc pipe %d-%d\n", getpid(), pp[0], pp[1]);
        printf("prime %d\n", first);

        int cp[2];
        pipe(cp); // cur new pipe

        if (fork()) { // fork child
            close(cp[0]); // close cur read

            int other;
            while (read(pp[0], &other, 4)) { // cur rev others
                if ((other % first) != 0) { // drop if times-of-first
                    write(cp[1], &other, 4); // cur write one by one, with child receive one by one for unbuffered char
                }
            }

            close(cp[1]); // close cur write
            wait(0);
            exit(0);
        }

        // child process
        close(pp[0]); // close prev read
        proc(cp); // recurive process cur pipe
        exit(0);
    }

    exit(0); // all recv, exit
}

int main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    if (fork()) {
        close(p[0]); // close cur read

        // printf("@%d proc pipe %d-%d\n", getpid(), p[0], p./[1]);
        printf("prime 2\n");
        for (int i = 3; i < 35; i++) {
            if ((i & 1) != 0) { // write primes one by one
                write(p[1], &i, 4); // writer(parent) should be blocked until reader(child) finish
            }
        }

        close(p[1]); // end cur write, close
        wait(0);
        exit(0);
    }

    proc(p);
    exit(0);
}