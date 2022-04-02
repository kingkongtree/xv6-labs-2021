#include "kernel/types.h"
#include "user/user.h"

static int addsll_2(int rs1, int rs2)
{
    int rd;
    __asm__ __volatile__ (
       ".insn r 0x1b, 0, 2, %[rd], %[rs1], %[rs2]"
             :[rd]"=r"(rd)
             :[rs1]"r"(rs1),[rs2]"r"(rs2)
     );
    
    return rd; 
}

static int orsra_4(int rs1, int rs2)
{
    int rd;
    __asm__ __volatile__ (
       ".insn r 0x1b, 2, 20, %[rd], %[rs1], %[rs2]"
             :[rd]"=r"(rd)
             :[rs1]"r"(rs1),[rs2]"r"(rs2)
     );
    
    return rd; 
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(2, "uptime with invalid argc %d\n", argc);
        exit(1);
    }

    int r1 = 3, r2 = 2;
    printf("addshf,3,2,sll #2 =%d\n", addsll_2(r1,r2));
    printf("orshf,3,2,sra #4 =%d\n", orsra_4(r1,r2));

    int ret = uptime();
    printf("%d\n", ret);
    exit(0);
}