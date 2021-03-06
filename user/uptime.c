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
static int addsll_2(int rs1, int rs2)
{
    int rd;
    __asm__ __volatile__ (
       ".insn r 0x1b, 6, 2, %[rd], %[rs1], %[rs2]"
             :[rd]"=r"(rd)
             :[rs1]"r"(rs1),[rs2]"r"(rs2)
     );
    
    return rd;
}

static int orsra_4(int rs1, int rs2)
{
    int rd;
    __asm__ __volatile__ (
       ".insn r 0x1b, 2, 68, %[rd], %[rs1], %[rs2]"
             :[rd]"=r"(rd)
             :[rs1]"r"(rs1),[rs2]"r"(rs2)
     );
    
    return rd;
}


#define INSN16(value)                               \
        __asm__ __volatile__ (".2byte "#value);     \

#define INSN(value)                                 \
        __asm__ __volatile__ (".word "#value);      \

#define INSN64(value)                               \
        __asm__ __volatile__ (".8byte "#value);     \

void print_gprs(void)
{
    // register int x0 asm("zero");
    // register int x1 asm("ra");
    register int x2 asm("sp");
    // register int x3 asm("gp");
    // register int x4 asm("tp");
    // register int x5 asm("t0");
    // register int x6 asm("t1");
    // register int x7 asm("t2");
    register int x8 asm("s0");
    register int x9 asm("s1");
    // register int x10 asm("a0");
    // register int x11 asm("a1");
    // register int x12 asm("a2");
    // register int x13 asm("a3");
    // register int x14 asm("a4");
    // register int x15 asm("a5");
    // register int x16 asm("a6");
    // register int x17 asm("a7");
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
    // register int x28 asm("t3");
    // register int x29 asm("t4");
    // register int x30 asm("t5");
    // register int x31 asm("t6");

    // printf(" - x0=zero= %d\n - x3=gp= 0x%x\n - x4=tp= 0x%x\n",
    //         x0, x3, x4);

    // printf(" - x1=ra= 0x%x\n - x5:7=t0:2= %d, %d, %d\n - x28:31=t3:6= %d, %d, %d, %d\n",
    //         x1, x5, x6, x7, x28, x29, x30, x31);
    // printf(" - x10:17=a0:7= %d, %d, %d, %d, %d, %d, %d, %d\n",
    //         x10, x11, x12, x13, x14, x15, x16, x17);

    printf(" - x2=sp= 0x%x\n - x8=fp= 0x%x\n - x9=s1= %d\n - x18:x27=s2:11= %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            x2, x8, x9, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27);
}

/*
 * stmia: 
 * +---+---------------+------+-----+-----------+---------+
 * | e | mask[15:5]    | rs1  | 001 | mask[4:0] | 0001011 | // stmia
 * | e | mask[15:5]    | rs1  | 000 | mask[4:0] | 0001011 | // ldmia
 * +---+---------------+------+-----+-----------+---------+
 * 31  30              19     14    11          6         0
 */
void test_mia(void)
{
    register int x2 asm("sp");
    register int x1 asm("ra");
    register int x8 asm("s0");
    printf(" - x2=sp= 0x%x\n - x1=ra= 0x%x\n - x8=fp= 0x%x\n", x2, x1, x8);

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

    printf("\n ====>ldmia {x27} (sp)\n");
    print_gprs();
    INSN(0x0001008b) // x27 = s11

    printf("\n ====>ldmia {x23-x27} (sp)\n");
    print_gprs();
    INSN(0x00010f8b) // x23-x27 = s7-s11

    printf("\n ====>ldmia {x19-x27} (sp)\n");
    print_gprs();
    INSN(0x00f10f8b) // x19-x27 = s3-s11

    printf("\n ====>ldmia {x18-27} (sp)\n");
    print_gprs();
    INSN(0x01f10f8b) // x18-x27 =s2-s11

    printf("\n ====>stmia {x27} (sp)\n");
    print_gprs();
    INSN(0x0001108b) // x27 = s11

    printf("\n ====>stmia {x23-x27} (sp)\n");
    print_gprs();
    INSN(0x00011f8b) // x23-x27 = s7-s11

    printf("\n ====>stmia {x19-27} (sp)\n");
    print_gprs();
    INSN(0x00f11f8b) // x19-x27 = s3-s11

    printf("\n ====>stmia {x18-27} (sp)\n");
    print_gprs();
    INSN(0x01f11f8b) // x18-x27 =s2-s11

    // printf("\n ====>stmia {x9, x18-27} (sp)\n");
    // print_gprs();
    // INSN(0x03f12f8b) // x9, x18-x27 = s1, s2-s11

    // printf("\n ====>stmia {x8-x9, x18-27} (sp)\n");
    // print_gprs();
    // INSN(0x07f12f8b) // x8-x9, x18-x27 = s0-s1, s2-s11

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

    printf("\n ====>ldmia {x27} (sp)\n");
    print_gprs();
    INSN(0x0001008b) // x27 = s11

    printf("\n ====>ldmia {x23-x27} (sp)\n");
    print_gprs();
    INSN(0x00010f8b) // x23-x27 = s7-s11

    printf("\n ====>ldmia {x19-x27} (sp)\n");
    print_gprs();
    INSN(0x00f10f8b) // x19-x27 = s3-s11

    printf("\n ====>ldmia {x18-27} (sp)\n");
    print_gprs();
    INSN(0x01f10f8b) // x18-x27 =s2-s11

    // printf("\n ====>ldmia {x9, x18-27} (sp)\n");
    // print_gprs();
    // INSN(0x03f10f8b) // x9, x18-x27 = s1, s2-s11

    // printf("\n ====>ldmia {x8-x9, x18-x27} (sp)\n");
    // print_gprs();
    // INSN(0x07f10f8b) // x8-x9, x18-x27 = s0-s1, s2-s11
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
    register int x2 asm("sp");
    register int x1 asm("ra");
    register int x8 asm("s0");
    printf("\n - x2=sp= 0x%x\n - x1=ra= 0x%x\n - x8=fp= 0x%x\n", x2, x1, x8);

    printf("\n ====> prefi s1 #0x15\n");
    INSN(0x5404a00b) // prefi
    printf("\n ====> prefd s1 #0x15\n");
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
    register int x2 asm("sp");
    register int x1 asm("ra");
    register int x8 asm("s0");
    printf(" - x2=sp= 0x%x\n - x1=ra= 0x%x\n - x8=fp= 0x%x\n", x2, x1, x8);

    register int x9 asm("s1");

    printf("\n - x9=s1= 0x%x\n", x9);

    printf("\n ====> li s1,#0xdeadbeaf\n");

    // ???8??????????????????16bit??????, 852a=mov a0,a0, ??????????????????????????????
    INSN64(0x852adeadbeaf049f)

    printf("\n - x9=s1= 0x%x\n", x9);
}

/*
 * c.utxb/c.utxh: b5:2 from 00/01 to 10/11 for overlap
 * +--------+-----+----+-------+
 * | 100111 | rs1 | 10 | 00001 | // c.utxb
 * | 100111 | rs1 | 11 | 00001 | // c.utxh
 * +--------+-----+----+-------+
 * 15       10    7    5       0
 */
void test_c_utx(void)
{
    register int x2 asm("sp");
    register int x1 asm("ra");
    register int x8 asm("s0");
    printf(" - x2=sp= 0x%x\n - x1=ra= 0x%x\n - x8=fp= 0x%x\n", x2, x1, x8);

    register int x9 asm("s1");
    __asm__ __volatile__ ("li s1, 0xabcdef01");
    printf("\n - x9=s1= 0x%x\n", x9);

    printf("\n ====> c.utxb s1\n");
    INSN16(0x9cc1) // c.utxb
    printf("\n - x9=s1= 0x%x\n", x9);

    __asm__ __volatile__ ("li s1, 0xabcdef01");
    printf("\n - x9=s1= 0x%x\n", x9);

    printf("\n ====> c.utxh s1\n");
    INSN16(0x9ce1) // c.utxh
    printf("\n - x9=s1= 0x%x\n", x9);
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
    __asm__ __volatile__ ("li a4, 0xabcdef98");
    __asm__ __volatile__ ("addi a5,x2,0");

    INSN16(0xab98) // c.sb a4, #16(a5)

    __asm__ __volatile__ ("li a4, 0xdeadbeaf");
    __asm__ __volatile__ ("addi a5,x2,0");

    INSN16(0x2b98) // c.lbu
}

/*
 * c.lbh/c.sh:
 * +-----+---------+-----------+-----+-----------+----+----+
 * | 001 | uimm[0] | uimm[4:3] | rs1 | uimm[2:1] | rd | 10 | // c.lbu
 * | 101 |  imm[0] |  imm[4:3] | rs1 |  imm[2:1] | rd | 10 | // c.sb
 * +-----+---------+-----------+-----+-----------+----+----+
 * 15    12        11          9     6           4    1    0
 */
void test_c_lhu_sh(void)
{
    __asm__ __volatile__ ("li a4, 0xabcdef76");
    __asm__ __volatile__ ("addi a5,x2,0");

    INSN16(0xab9a) // c.sh a4, #16(a5)

    __asm__ __volatile__ ("li a4, 0xdeadbeaf");
    __asm__ __volatile__ ("addi a5,x2,0");

    INSN16(0x2b9a) // c.lhu a4, #16(a5)
}

/*
 * bcondi:
 * +--------+-------------+-------+-----+-------------+---------+
 * | cmp7   | offset[9:6] | rs1   | 000 | offset[5:1] | 0111011 | // beqi
 * | cmp7   | offset[9:6] | rs1   | 001 | offset[5:1] | 0111011 | // bnei
 * | cmp7   | offset[9:6] | rs1   | 100 | offset[5:1] | 0111011 | // blti
 * | cmp7   | offset[9:6] | rs1   | 101 | offset[5:1] | 0111011 | // bgei
 * | cmp7   | offset[9:6] | rs1   | 110 | offset[5:1] | 0111011 | // bltui
 * | cmp7   | offset[9:6] | rs1   | 111 | offset[5:1] | 0111011 | // bgeui
 * +--------+-------------+-------+-----+-------------+---------+
 * 31       24            19      14    11            6         0
 */
void test_bcondi(void)
{
    register int x27 asm("s11");

    __asm__ __volatile__ ("addi s11, x0, 0x70");
    INSN(0x701d813b) // beqi s11,0x70, 68(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x69");
    INSN(0x700d9fbb) // bnei s11,0x70, 60(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x68");
    INSN(0x700dcd3b) // blti s11,0x70, 52(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x67");
    INSN(0x700ddb3b) // bgei s11,0x70, 44(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x66");
    INSN(0x700de93b) // bltui s11,0x70, 36(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x65");
    INSN(0x700df73b) // bgeui s11,0x70, 28(sp)

    printf("miss #1: rs1=x27=s11= 0x%x, cmpimm= 0x%x\n", x27, 0x70);
    printf("hit  #1: rs1=x27=s11= 0x%x, cmpimm= 0x%x\n", x27, 0x70);

    __asm__ __volatile__ ("addi s11, x0, 0x70");
    INSN(0x700d9fbb) // bnei s11,0x70, 60(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x69");
    INSN(0x700dcd3b) // blti s11,0x70, 52(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x68");
    INSN(0x700ddb3b) // bgei s11,0x70, 44(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x67");
    INSN(0x700de93b) // bltui s11,0x70, 36(sp)

    __asm__ __volatile__ ("addi s11, x0, 0x66");
    INSN(0x700df73b) // bgeui s11,0x70, 28(sp)

    printf("miss #2: rs1=x27=s11= 0x%x, cmpimm= 0x%x\n", x27, 0x70);
    printf("hit  #2: rs1=x27=s11= 0x%x, cmpimm= 0x%x\n", x27, 0x70);
}

/* 
 * muliadd:
 * +-----------+-------+-------+---------+----+------+---------+
 * | uimm[7:1] | rs2   | rs1   | uimm[0] | 01 | rd   | 1011011 |
 * +-----------+-------+-------+---------+----+------+---------+
 * 31          25      20      15        14   12     7         0
 */
void test_muliadd(void)
{
    register int x11 asm("a1");

    __asm__ __volatile__ ("addi s1, x0, 3");
    __asm__ __volatile__ ("addi s2, x0, 2");
    
    INSN(0x724d5db) // muliadd a1, s1, s2 #7
    
    printf("muliadd a1, 3, 2 #7 = %d\n", x11);
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
    // just verfiy it while gdb debugging _uptime
    //INSN16(0x8050) // c.pop {x1, x8-x9, x18-x19}, #0(sp)

    // just verfiy it while gdb debugging _uptime
    //INSN16(0x8268) // c.push {x1, x8-x9, x18-x20}, #2(sp)

    // may cause unexpected usertrap for random ret; verified also in c.pop
    // just verfiy it while gdb debugging trans_c_popret function for decoding
    // INSN16(0x8054) // c.popret {x1, x8-x9, x18-x19}, #0(sp)
}

/*
 * j16m/jal16m: 
 * +---------+-----------+---------+------------+------------+---+---------+
 * | imm[20] | imm[10:1] | imm[11] | imm[19:12] | imm[24:21] | 0 | 1111011 | // jal16m
 * | imm[20] | imm[10:1] | imm[11] | imm[19:12] | imm[24:21] | 1 | 1111011 | // j16m
 * +---------+-----------+---------+------------+------------+---+---------+
 * 31        30          20        19           12           8   7         0
 */
void test_j16m(void)
{
    // just verfiy it while gdb debugging trans_c_popret function for decoding
    // INSN(0x010101fb) // j16m #2162704

    // just verfiy it while gdb debugging trans_c_popret function for decoding
    // INSN(0x1010107b) // jal16m #6400
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(2, "uptime with invalid argc %d\n", argc);
        exit(1);
    }

    int ret = uptime();
    printf("%d\n", ret);

    register int x2 asm("sp");
    register int x1 asm("ra");
    register int x8 asm("s0");
    printf("\n - x2=sp= 0x%x\n - x1=ra= 0x%x\n - x8=fp= 0x%x\n", x2, x1, x8);

    printf("\n================ preshf test ===============\n");
    int r1 = 3, r2 = 2;
    printf("addshf,3,2,sll #2 =%d\n", addsll_2(r1,r2));
    printf("orshf,3,2,sra #4 =%d\n", orsra_4(r1,r2));

    printf("\n================ mia test ==================\n");
    test_mia();

    printf("\n================ prf test ==================\n");
    test_prf();

    printf("\n================ l.li test =================\n");
    test_l_li();

    printf("\n================ c.utx test ================\n");
    test_c_utx();

    printf("\n================ c.lbu/c.sb test ===========\n");
    test_c_lbu_sb();

    printf("\n================ c.lbh/c.sh test ===========\n");
    test_c_lhu_sh();

    printf("\n================ bcondi test ===============\n");
    test_bcondi();

    printf("\n================ muliadd test ==============\n");
    test_muliadd();

    printf("\n================ c.pop/c.push test =========\n");
    test_c_pop_push();

    printf("\n================ j16m/jal16m ===============\n");
    test_j16m();

    exit(0);
}
