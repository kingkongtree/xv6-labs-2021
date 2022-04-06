#include "kernel/types.h"
#include "user/user.h"

/*
 * R type: .insn r opcode, func3, func7, rd, rs1, rs2
 * +-------+-----+-----+-------+----+-------------+
 * | func7 | rs2 | rs1 | func3 | rd |      opcode |
 * +-------+-----+-----+-------+----+-------------+
 * 31      25    20    15      12   7             0
 */

/* 
 * preshf:
 * +------+-------+-------+-------+-----+------+---------+
 * | type |   amt |   rs2 |   rs1 | opc |   rd | 0011011 |
 * +------+-------+-------+-------+-----+------+---------+
 * 31     29      24      19      14    11     7         0
 */
int addsll_2(int rs1, int rs2)
{
    int rd;
    __asm__ __volatile__ (
       ".insn r 0x1b, 6, 2, %[rd], %[rs1], %[rs2]"
             :[rd]"=r"(rd)
             :[rs1]"r"(rs1),[rs2]"r"(rs2)
     );
    
    return rd;
}

int orsra_4(int rs1, int rs2)
{
    int rd;
    __asm__ __volatile__ (
       ".insn r 0x1b, 2, 68, %[rd], %[rs1], %[rs2]"
             :[rd]"=r"(rd)
             :[rs1]"r"(rs1),[rs2]"r"(rs2)
     );
    
    return rd;
}

#define INSN(value)                             \
        __asm__ __volatile__ (".word "#value);  \

void print_gprs(void)
{
    register int x0 asm("zero");
    register int x1 asm("ra");
    register int x2 asm("sp");
    register int x3 asm("gp");
    register int x4 asm("tp");
    register int x5 asm("t0");
    register int x6 asm("t1");
    register int x7 asm("t2");
    register int x8 asm("s0");
    register int x9 asm("s1");
    register int x10 asm("a0");
    register int x11 asm("a1");
    register int x12 asm("a2");
    register int x13 asm("a3");
    register int x14 asm("a4");
    register int x15 asm("a5");
    register int x16 asm("a6");
    register int x17 asm("a7");
    register int x18 asm("s2");
    register int x19 asm("s3");
    register int x20 asm("s4");
    register int x21 asm("s5");
    register int x22 asm("s6");
    register int x23 asm("s7");
    register int x24 asm("s8");
    register int x25 asm("s9");
    register int x26 asm("s10");
    register int x27 asm("s11");
    register int x28 asm("t3");
    register int x29 asm("t4");
    register int x30 asm("t5");
    register int x31 asm("t6");

    printf("system regs: \n - x0=zero= %d\n - x3=gp= 0x%x\n - x4=tp= 0x%x\n",
            x0, x3, x4);

    printf("caller regs: \n - x1=ra= 0x%x\n - x5:7=t0:2= %d, %d, %d\n - x28:31=t3:6= %d, %d, %d, %d\n",
            x1, x5, x6, x7, x28, x29, x30, x31);
    printf("callee regs: \n - x10:17=a0:7= %d, %d, %d, %d, %d, %d, %d, %d\n",
            x10, x11, x12, x13, x14, x15, x16, x17);

    printf("callee regs: \n - x2=sp= 0x%x\n - x8=fp= 0x%x\n - x9=s1= %d\n - x18:x27=s2:11= %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            x2, x8, x9, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27);
}

/*
 * ldmia: 
 * +---+---------------+------+-----+-----------+---------+
 * | e | mask[15:5]    | rs1  | 000 | mask[4:0] | 0001011 |
 * +---+---------------+------+-----+-----------+---------+
 * 31  30              19     14    11          6         0
 */
static inline void ldmia_sp(void)
{
    printf("==> ldmia-callee {x8-x9, x18-x27}, (sp)\n");

    print_gprs();

    // ldm_e = 0, gpr_mask=8190=0b1111111111110, rs1=2
    printf("==> cpu-exec 0x%x\n", 0x0ff10f0b);
    INSN(0x0ff10f0b)

    print_gprs();
}

/*
 * stmia: 
 * +---+---------------+------+-----+-----------+---------+
 * | e | mask[15:5]    | rs1  | 001 | mask[4:0] | 0001011 |
 * +---+---------------+------+-----+-----------+---------+
 * 31  30              19     14    11          6         0
 */
static inline void stmia_sp(void)
{
    printf("==> stmia-callee {x8-x9, x18-x27}, (sp)\n");

    print_gprs();

    // ldm_e = 0, gpr_mask=8190=0b1111111111110, rs1=2
    printf("==> cpu-exec 0x%x\n", 0x0ff11f0b);
    INSN(0x0ff11f0b)

    print_gprs();
}

void test_mia(void)
{
    __asm__ __volatile__ ("li s1, 11");
    __asm__ __volatile__ ("li s2, 12");
    __asm__ __volatile__ ("li s3, 13");
    __asm__ __volatile__ ("li s4, 14");
    __asm__ __volatile__ ("li s5, 15");
    __asm__ __volatile__ ("li s6, 16");
    __asm__ __volatile__ ("li s7, 17");
    __asm__ __volatile__ ("li s8, 18");
    __asm__ __volatile__ ("li s9, 19");
    __asm__ __volatile__ ("li s10, 20");
    __asm__ __volatile__ ("li s11, 21");

    stmia_sp();

    __asm__ __volatile__ ("li s1, 101");
    __asm__ __volatile__ ("li s2, 102");
    __asm__ __volatile__ ("li s3, 103");
    __asm__ __volatile__ ("li s4, 104");
    __asm__ __volatile__ ("li s5, 105");
    __asm__ __volatile__ ("li s6, 106");
    __asm__ __volatile__ ("li s7, 107");
    __asm__ __volatile__ ("li s8, 108");
    __asm__ __volatile__ ("li s9, 109");
    __asm__ __volatile__ ("li s10, 110");
    __asm__ __volatile__ ("li s11, 111");

    ldmia_sp();
}

/*
 * prf: 
 * +--------+--------+-------+-----+--------+------------+
 * | imm7   | 00000  | rs1   | 010 | 00000  | 0001011    | // prefi
 * | imm7   | 00000  | rs1   | 011 | 00000  | 0001011    | // prefd
 * +--------+--------+-------+-----+--------+------------+
 * 31       24       19      14    11       6            0
 */
void test_prf(void)
{
    INSN(0x5404a00b) // prefi
    INSN(0x5404b00b) // prefd
}

/*
 * l.li: 
 * +---------------+-------+------+------------+
 * | imm32         | 0000  | rd   | 0011111    |
 * +---------------+-------+------+------------+
 * 47              15      11     6            0
 */
void test_l_li(void)
{
    __asm__ __volatile__ (".8byte 0x0000beaf049f");
}

/*
 * c.lbu/c.sb: 
 * +-----+---------+-----------+-----+-----------+----+----+
 * | 001 | uimm[0] | uimm[4:3] | rs1 | uimm[2:1] | rd | 00 | // c.lbu
 * | 101 |  imm[0] |  imm[4:3] | rs1 |  imm[2:1] | rd | 00 | // c.sb
 * +-----+---------+-----------+-----+-----------+----+----+
 * 15    12        11          9     6           4    1    0
 */
void test_c_lbu_sb(void)
{
    __asm__ __volatile__ (".2byte 0xac64"); // c.sb
    __asm__ __volatile__ (".2byte 0x2c64"); // c.lbu
}

/*
 * c.pop/c.popret/c.push: 
 * +-----+---------+--------+----+----+
 * | 100 | sp16imm | rcount | 00 | 00 | // c.pop
 * | 100 | sp16imm | rcount | 01 | 00 | // c.popret
 * | 100 | sp16imm | rcount | 10 | 00 | // c.push
 * +-----+---------+--------+----+----+
 * 15    12        7        3    1    0
 */
void test_c_pop_push(void)
{
    __asm__ __volatile__ (".2byte 0x8a30"); // c.pop
    __asm__ __volatile__ (".2byte 0x8a3c"); // c.push
    __asm__ __volatile__ (".2byte 0x8a34"); // c.popret
}

/*
 * bxxi: opc change from 0111011 to 0111111 for overlap
 * +--------+-------------+-------+-----+-------------+---------+
 * | cmp7   | offset[9:6] | rs1   | 000 | offset[5:1] | 0111111 | // beqi
 * | cmp7   | offset[9:6] | rs1   | 001 | offset[5:1] | 0111111 | // bnei
 * | cmp7   | offset[9:6] | rs1   | 100 | offset[5:1] | 0111111 | // blti
 * | cmp7   | offset[9:6] | rs1   | 101 | offset[5:1] | 0111111 | // bgei
 * | cmp7   | offset[9:6] | rs1   | 110 | offset[5:1] | 0111111 | // bltui
 * | cmp7   | offset[9:6] | rs1   | 111 | offset[5:1] | 0111111 | // bgeui
 * +--------+-------------+-------+-----+-------------+---------+
 * 31       24            19      14    11            6         0
 */
void test_bcondi(void)
{
    INSN(0x640481bf) // beqi
    INSN(0x640491bf) // bnei
    INSN(0x6404c1bf) // blti
    INSN(0x6404d1bf) // bgei
    INSN(0x6404e1bf) // bltui
    INSN(0x6404f1bf) // bgeui
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(2, "uptime with invalid argc %d\n", argc);
        exit(1);
    }

    int ret = uptime();
    printf("%d\n", ret);

    printf("\n================ preshf test ===========\n");
    int r1 = 3, r2 = 2;
    printf("addshf,3,2,sll #2 =%d\n", addsll_2(r1,r2));
    printf("orshf,3,2,sra #4 =%d\n", orsra_4(r1,r2));

    printf("\n================ prf test ===========\n");
    test_prf();

    printf("\n================ c.lbu/c.sb test ===========\n");
    test_c_lbu_sb();

    // what code done, but debug todo
    printf("\n================ mia test ===========\n");
    test_mia();

    printf("\n================ l.li test ===========\n");
    test_l_li();

    printf("\n================ prf c.pop/c.push ===========\n");
    test_c_pop_push();

    printf("\n================ bcondi test ===========\n");
    test_bcondi();

    exit(0);
}
