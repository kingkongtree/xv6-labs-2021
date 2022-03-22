#include "kernel/types.h"
#include "user/user.h"

static int custom_cube(uint64 addr)
{
    int cube;
    asm volatile (
       ".insn r 0x7b, 6, 6, %0, %1, x0"
             :"=r"(cube)
             :"r"(addr)
     );
    return cube; 
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(2, "uptime with invalid argc %d\n", argc);
        exit(1);
    }

    int a = 3, cube = 0;
    cube = custom_cube((uint64)&a);
    printf("cube=%d\n", cube);

    int ret = uptime();
    printf("%d\n", ret);
    exit(0);
}