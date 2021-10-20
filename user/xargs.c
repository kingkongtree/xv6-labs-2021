#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int xargs(char *cmd, int argc, char *argv[], char *line)
{
    if (fork()) {
        wait(0);
        return 0;
    }

    // cargv[0] == cmd, cargv[-1] = 0
    char *cargv[MAXARG];
    for (int i = 0; i < argc; i++) cargv[i] = argv[i];
    cargv[argc] = line;
    cargv[argc + 1] = 0;
    exec(cmd, cargv);

    return 0;
}

int main(int argc, char *argv[])
{
    char line[512];
    char *pc = line;

    if (argc < 2 || argc > MAXARG) {
        fprintf(2, "invalid argc %d\n", argc);
        exit(1);
    }

    while (read(0, pc, 1)) {
        if (*pc == '\n') {
            *pc++ = '\0';
            xargs(argv[1], argc - 1, &(argv[1]), line);
            pc = line;
            continue;
        }
        pc++;
    }

    exit(0);
}