# 1. [官方文档](https://qemu.readthedocs.io/en/latest/devel/index.html)
- [Documentation/TCG](https://wiki.qemu.org/Documentation/TCG)
    - [intro README](https://gitlab.com/qemu-project/qemu/-/blob/master/tcg/README) 概述
    - [docs/devel/tcg.rst](https://gitlab.com/qemu-project/qemu/-/blob/master/docs/devel/tcg.rst) 语法、基础ir
    - [decodetree](https://gitlab.com/qemu-project/qemu/-/blob/master/docs/devel/decodetree.rst) 指令格式的抽象描述，pattern
        - .decode: 定义一个新的独立的.decode文件
        - decodetree.py: python实现的对insn pattern->c的翻译
            - getopt,解析入参。o-output，w-width
            - parse_file，剔除注释，闭合括号，分别匹配field/arguments/format/pattern
            - build_tree, 重建pattern间的关联，合法性及重复检查
        - .meson: python的配置文件,指明指令宽度（映射成insnmask）
            ```
            gen = [
                decodetree.process('insn16.decode', extra_args: ['--static-decode=decode_insn16', '--insnwidth=16']),
                decodetree.process('insn32.decode', extra_args: '--static-decode=decode_insn32'),
            ]
            ```
    - [backend ops](https://wiki.qemu.org/Documentation/TCG/backend-ops) qemu官方定义的原子指令，后端表示即IR
    - [frontend ops](https://wiki.qemu.org/Documentation/TCG/frontend-ops) 后端op的C表示
- [riscv-isa-manual](https://github.com/riscv/riscv-isa-manual)
    - [RISC-V Privileged ISA v1.12](https://github.com/riscv/riscv-isa-manual/releases/download/Priv-v1.12/riscv-privileged-20211203.pdf)
    - [RISC-V Unprivileged ISA v2.1](https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf)
- [ARM](https://developer.arm.com/documentation?_ga=2.223438683.1363885975.1647911959-1057699989.1647911959#cf[navigationhierarchiesproducts]=Architectures,CPU%20Architecture)
    - [Arm Armv9-A A64](https://documentation-service.arm.com/static/61c04cbab691546d37bd2b87?token=)
    - [Arm Armv8-A A32/T32](https://documentation-service.arm.com/static/61c04ba12183326f217711e0?token=)
# 2. 参考文档
- 教程
    - [riscv gcc中添加custom自定义指令](https://cloud.tencent.com/developer/article/1886469)
    - [riscv实现自定义指令并用qemu运行](https://cloud.tencent.com/developer/article/1819855?from=article.detail.1770528)
    - [qemu 概念空间](https://gitee.com/Kenneth-Lee-2012/MySummary/blob/master/概念空间分析/qemu.rst#id2)
- 参考实现
    - [plct + nuclei](https://link.zhihu.com/?target=https%3A//github.com/isrc-cas/plct-qemu/tree/plct-nuclei)
    - [plct + sifive](https://github.com/plctlab/plct-qemu/tree/rvv-sifive-rfc-rc3)
    - [sifive - freedom](https://github.com/sifive/freedom-qemu)
    - [xilinx qemu](https://github.com/Xilinx/qemu)
    - [南大 nemu](https://github.com/NJU-ProjectN/nemu)
- isa spec
    - [nuclei sdk](https://github.com/Nuclei-Software/nuclei-sdk)
    - [nuclei n](https://www.riscv-mcu.com/quickstart-doc-u-nuclei_n_isa.html)
    - [nuclei e200](https://github.com/SI-RISCV/e200_opensource)
    - [nutshell](https://github.com/OSCPU/NutShell)
# 3. vscode配置调试
- [新增arch](https://github.com/kingkongtree/xv6-labs-2021/blob/main/qemu/add_arch.md)
- [xv6 over qemu分析](https://github.com/kingkongtree/xv6-labs-2021/blob/main/qemu/xv6_over_qemu.md)
- task.json
    ```
        {
            "label": "build",
            "type": "shell",
            "command": "cd build; ../configure --enable-debug --target-list=tree-softmmu; make -j8",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
    ```
- launch.json
    ```
    {
            "type": "lldb",
            "request": "launch",
            "name": "Debug xv6",
            "program": "${workspaceFolder}/build/qemu-system-tree",
            "args": [
                "-machine", "virt",
                "-bios", "none",
                "-kernel", "../xv6-labs-2021-my/kernel/kernel",
                "-m", "128M",
                //"-smp", "3",
                "-nographic",
                "-drive", "file=../xv6-labs-2021-my/fs.img,if=none,format=raw,id=x0",
                "-device", "virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0",
                //"-d", "in_asm", "-D", "in_asm.log",
                //"-d", "out_asm", "-D", "out_asm.log",
                "-d", "op", "-D", "op.log"
            ],
            "cwd": "${workspaceFolder}",
            //"preLaunchTask": "build"
    },
    ```
# 3. preshft指令
- spec定义 
    ```
    shftype[31,30] + shfamt[29~25] + rs2[24~20] + rs1[19~15] + shfopc[14~12] + rd[11~7] + opc[0011011]
    ```
- 后端实现：decodetree pattern
    - field: rd,rs2,rs1都是标准定义，shfopc在shfxxx指令中直接定义
        ```
        %shftype    30:2
        %shfamt     25:5
        ```
    - argset: 这里参数顺序体现的是汇编定义
        ```
        &preshf     rd rs1 rs2 shftype shfamt
        ```
    - format: 这里参数顺序体现的是机器码定义
        ```
        @preshf     .. ..... ..... ..... ... ..... ....... &preshf      %shftype %shfamt %rs2 %rs1 %rd
        ```
    - insn: 根据shfopc不同，同一format可以编码成不同指令
        > overlap的指令，将公版的/高频的放在前面
        ```
        {
          addiw       ............   ..... 000 ..... 0011011 @i
          add_preshf  .. ..... ..... ..... 000 ..... 0011011 @preshf
        }

        {
          slliw       0000000 .....  ..... 001 ..... 0011011 @sh5
          clzw        0110000 00000 ..... 001 ..... 0011011 @r2
          ctzw        0110000 00001 ..... 001 ..... 0011011 @r2
          cpopw       0110000 00010 ..... 001 ..... 0011011 @r2
          bsetiw      0010100 .......... 001 ..... 0011011 @sh5
          bclriw      0100100 .......... 001 ..... 0011011 @sh5
          binviw      0110100 .......... 001 ..... 0011011 @sh5
          sloiw       0010000 .......... 001 ..... 0011011 @sh5
          slli_uw     00001. ........... 001 ..... 0011011 @sh
          sub_preshf  .. ..... ..... ..... 001 ..... 0011011 @preshf
        }

        or_preshf   .. ..... ..... ..... 010 ..... 0011011 @preshf
        xor_preshf  .. ..... ..... ..... 011 ..... 0011011 @preshf
        and_preshf  .. ..... ..... ..... 100 ..... 0011011 @preshf
        ```
- 前端实现：c translate
    ```
    #define GEN_TRANS_PRESHF(SHFOPC)                                                    \
    static bool trans_##SHFOPC##_preshf(DisasContext *ctx, arg_##SHFOPC##_preshf *a)    \
    {                                                                                   \
        TCGv shfdst = tcg_temp_new();                                                   \
        TCGv shfamt = tcg_temp_new();                                                   \
        TCGv shfsrc = tcg_temp_new();                                                   \
        TCGv dst = tcg_temp_new();                                                      \
        TCGv src = tcg_temp_new();                                                      \
                                                                                        \
        gen_get_gpr(shfsrc, a->rs2);                                                    \
        tcg_gen_movi_tl(shfamt, a->shfamt);                                             \
                                                                                        \
        switch(a->shftype) { /* by shftype */                                           \
            case 0: /* 00, sll */                                                       \
                tcg_gen_shl_tl(shfdst, shfsrc, shfamt);                                 \
                break;                                                                  \
            case 1: /* 01, srl */                                                       \
                tcg_gen_shr_tl(shfdst, shfsrc, shfamt);                                 \
                break;                                                                  \
            case 2: /* 10, sra */                                                       \
                tcg_gen_sar_tl(shfdst, shfsrc, shfamt);                                 \
                break;                                                                  \
            case 3: /* 11, ror */                                                       \
                tcg_gen_rotr_tl(shfdst, shfsrc, shfamt);                                \
                break;                                                                  \
            default:                                                                    \
                return false;                                                           \
        }                                                                               \
                                                                                        \
        gen_get_gpr(dst, a->rd);                                                        \
        gen_get_gpr(src, a->rs1);                                                       \
                                                                                        \
        tcg_gen_##SHFOPC##_tl(dst, src, shfdst); /* by shfopc */                        \
                                                                                        \
        gen_set_gpr(a->rd, dst);                                                        \
                                                                                        \
        tcg_temp_free(shfdst);                                                          \
        tcg_temp_free(shfsrc);                                                          \
        tcg_temp_free(shfamt);                                                          \
        tcg_temp_free(src);                                                             \
        tcg_temp_free(dst);                                                             \
                                                                                        \
        return true;                                                                    \
    }                                                                                   \

    GEN_TRANS_PRESHF(add)
    GEN_TRANS_PRESHF(sub)
    GEN_TRANS_PRESHF(or)
    GEN_TRANS_PRESHF(xor)
    GEN_TRANS_PRESHF(and)
    ```
# 4. 通过gnu-as内联汇编构造用例
- [RISC-V Directives](https://sourceware.org/binutils/docs-2.33.1/as/RISC_002dV_002dDirectives.html#RISC_002dV_002dDirectives)
- [Instruction Formats](https://sourceware.org/binutils/docs-2.33.1/as/RISC_002dV_002dFormats.html#RISC_002dV_002dFormats) insn是gcc-riscv的俚语，qemu中也仅在RV中使用。
- [CSDN](https://www.cnblogs.com/xphh/p/11491681.html)
    ```
    // addshf,rd,rs1,rs2,sll #2 => asm (".insn r 0x7b, 0, 2, rd, rs1, rs2")
    static int addsll_2(int rs1, int rs2)
    {
        int rd;
        __asm__ __volatile__ (
        ".insn r 0x7b, 0, 2, %[rd], %[rs1], %[rs2]"
                :[rd]"=r"(rd)
                :[rs1]"r"(rs1),[rs2]"r"(rs2)
        );
        
        return rd; 
    }

    // orshf,rd,rs1,rs2,sra #4 => asm (".insn r 0x7b, 2, 20, rd, rs1, rs2")
    static int orsra_4(int rs1, int rs2)
    {
        int rd;
        __asm__ __volatile__ (
        ".insn r 0x7b, 2, 20, %[rd], %[rs1], %[rs2]"
                :[rd]"=r"(rd)
                :[rs1]"r"(rs1),[rs2]"r"(rs2)
        );
        
        return rd; 
    }

    #define INSN(value)                             \
        __asm__ __volatile__ (".word "#value);      \

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
    ```
# 5. LDMIA
- [LDM-LDMIA-LDMFD--Thumb](https://developer.arm.com/documentation/ddi0406/cb/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-instructions/LDM-LDMIA-LDMFD--Thumb-)
- [Arm Armv8-A A32/T32](https://documentation-service.arm.com/static/61c04ba12183326f217711e0?token=) P155
- [CSDN 介绍](https://www.cnblogs.com/lifexy/p/7363208.html)
- ARM-QEMU实现
    - [decodetree-t32](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/t32.decode)
    - [decodetree-a32](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/a32.decode)
    - [translate.c](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/translate.c)
- 实现
    ```
    %ldm_e      31:1
    %gpr_mask   20:11 7:5

    &ldm    ldm_e rs1 gpr_mask
    @ldm    . ........... ..... ... ..... ....... &ldm  %ldm_e %gpr_mask %rs1

    ldmia   . ........... ..... 000 ..... 0001011 @ldm

    static bool trans_ldmia(DisasContext *s, arg_ldmia *a)
    {
        int callee_reg_list[16] = { 2, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1, -1, -1 };
        int caller_reg_list[16] = { 1, 5, 6, 7,  10, 11, 12, 13, 14, 15, 16, 17, 28, 29, 30, 31 };
        int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
        unsigned int mask = a->gpr_mask;

        TCGv target_mem_addr = tcg_temp_new();
        TCGv src_reg_index = tcg_temp_new();

        gen_get_gpr(target_mem_addr, a->rs1);

        for (int i = 0; i < 16; ++i) {
            if (reg_list[i] == -1) break; // if meet undef reg, ignore all left.
            if (!(mask & (1 << i))) continue; // check if masked
            tcg_gen_qemu_ld_tl(src_reg_index, target_mem_addr, s->mem_idx, MO_TEQ); // port from trans_ld
            gen_set_gpr(reg_list[i], src_reg_index);
            tcg_gen_addi_tl(target_mem_addr, target_mem_addr, 8);
        }

        tcg_temp_free(target_mem_addr);
        tcg_temp_free(src_reg_index);

        return true;
    }
    ```
- 验证
    ```
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
    ```
# 6. STMIA
- [STM-STMIA-STMEA--Thumb](https://developer.arm.com/documentation/ddi0406/cb/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-instructions/STM--STMIA--STMEA-)
- [Arm Armv8-A A32/T32](https://documentation-service.arm.com/static/61c04ba12183326f217711e0?token=) P480
- [CSDN 介绍](https://www.cnblogs.com/lifexy/p/7363208.html)
- ARM-QEMU实现
    - [decodetree-t32](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/t32.decode)
    - [decodetree-a32](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/a32.decode)
    - [translate.c](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/translate.c)
- 实现
    ```
    stmia   . ........... ..... 001 ..... 0001011 @ldm

    static bool trans_stmia(DisasContext *s, arg_stmia *a)
    {
        int callee_reg_list[16] = { 2, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1, -1, -1 };
        int caller_reg_list[16] = { 1, 5, 6, 7,  10, 11, 12, 13, 14, 15, 16, 17, 28, 29, 30, 31 };
        int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
        unsigned int mask = a->gpr_mask;

        TCGv target_mem_addr = tcg_temp_new();
        TCGv src_reg_index;

        gen_get_gpr(target_mem_addr, a->rs1);

        for (int i = 0; i < 16; ++i) {
            if (reg_list[i] == -1) break; // if meet undef reg, ignore all left.
            if (!(mask & (1 << i))) continue; // check if masked

            src_reg_index = tcg_temp_new(); // renew temp once

            gen_get_gpr(src_reg_index, reg_list[i]);
            tcg_gen_qemu_st_tl(src_reg_index, target_mem_addr, s->mem_idx, MO_TEQ); // port from trans_sd
            tcg_gen_addi_tl(target_mem_addr, target_mem_addr, 8); // 64bit / 8 = 8
        
            tcg_temp_free(src_reg_index);
        }

        tcg_temp_free(target_mem_addr);

        return true;
    }
    ```
- 验证
    ```
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
    ```
# 7. PREFI/PREFD
- 直接打桩为nop
```
# *** perfi/prefd ***
%prf_imm7   25:7
&prf    prf_imm7 rs1
@prf    ....... ..... ..... ... ..... ....... &prf  %prf_imm7 %rs1

prefi   ....... 00000 ..... 010 00000 0001011 @prf
prefd   ....... 00000 ..... 011 00000 0001011 @prf

static bool trans_prefi(DisasContext *s, arg_prefi *a)
{
    return true;
}

static bool trans_prefd(DisasContext *s, arg_prefd *a)
{
    return true;
}
```
- 验证
```
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
```
# 8. L.LI
- qemu内部的IR本身支持32bit立即数加载，trans函数仿照lui实现即可
- decodetree仅支持16/32/64三种指令长度，因此需要修改decode_opc函数后，仿照decodetree实现解码。
- 实现
    - ./target/tree/insn_trans/decode-tree48.c.inc
        ```
        /* This file is porting from auto-generated decode-tree32.c.inc scripts/decodetree.py.  */

        /* insn arg_set */
        typedef struct {
            //
        } arg_l_li;

        /* insn trans function */
        static bool trans_l_li(DisasContext *s, arg_l_li *a)
        {
            //
        }

        /* insn extract func */
        static void decode_tree48_extract_l_li(DisasContext *ctx, arg_l_li *a, uint64_t insn)
        {
            //
        }

        /* main decode func */
        static bool decode_tree48(DisasContext *ctx, uint64_t insn)
        {
            union {
                arg_l_li f_l_li;
            } u;

            switch (insn & 0x0000007f) {
                case 0x0000001f:
                    /* ........ ........ ........ ........ .0011111 */
                    decode_tree48_extract_l_li(ctx, &u.f_l_li, insn);
                    switch ((insn >> 12) & 0xf) {
                        case 0x0:
                            /* ........ ........ ........ 0000.... .0011111 */
                            if (trans_l_li(ctx, &u.f_l_li)) return true;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

            return false;
        }
        ```
- 验证
```
/*
 * l,li: 
 * +---------------+-------+------+------------+
 * | imm32         | 0000  | rd   | 0011111    |
 * +---------------+-------+------+------------+
 * 47              15      11     6            0
 */
void test_l_li(void)
{
    __asm__ __volatile__ (".8byte 0x0000beaf049f");
}
```
# 9. C.LBU/C.SB
- 实现
```
%uimm_cl_tree     5:2 10:3           !function=ex_tree_cl
@cl_tree      ... ... ... .. ... .. &i      imm=%uimm_cl_tree   rs1=%rs1_3  rd=%rs2_3
@cs_tree      ... ... ... .. ... .. &s      imm=%uimm_cl_tree   rs1=%rs1_3  rs2=%rs2_3
# *** c.lbu / c.sb ***
{
  c_lbu           001  ... ... .. ... 00 @cl_tree
  fld             001  ... ... .. ... 00 @cl_d
}

{
  c_sb            101  ... ... .. ... 00 @cs_tree
  fsd             101  ... ... .. ... 00 @cs_d
}

static int ex_tree_cl(DisasContext *ctx, int imm)
{
    /* 2-1-0-4-3 -> 0-4-3-2-1 convertion */
    unsigned int b0 = imm & 0x1;
    unsigned int b1 = (imm >> 1) & 0x1;
    unsigned int b2 = (imm >> 2) & 0x1;
    unsigned int b3 = (imm >> 3) & 0x1;
    unsigned int b4 = (imm >> 4) & 0x1;
    
    return (b1 << 4) | (b0 << 3) | (b4 << 2) | (b3 << 1) | b2;
}

static bool trans_c_lbu(DisasContext *ctx, arg_i *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm);

    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, MO_UB);
    gen_set_gpr(a->rd, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_sb(DisasContext *ctx, arg_s *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm);
    gen_get_gpr(dat, a->rs2);

    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, MO_SB);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}
```
- 验证
# 10. C.POP/C.PUSH/C.POPRET
- 参考RX ISA实现
    - ./target/tree/tree32.decode
    ```
    # argument set should define in tree32.decode, and extern to tree16
    &c_push      c_sp16imm c_rcount
    ```
    - ./target/tree/tree16.decode
    ```
    # *** c.pop / c.popret / c.push ***
    %c_sp16imm  8:5
    %c_rcount   4:4

    # argument set should define in tree32.decode, and extern to tree16
    &c_push      c_sp16imm c_rcount   !extern
    @c_push      ... ..... .... .. .. &c_push  %c_sp16imm %c_rcount

    c_pop       100 ..... .... 00 00 @c_push
    c_popret    100 ..... .... 01 00 @c_push
    c_push      100 ..... .... 10 00 @c_push
    ```
    -
    ```

    static bool trans_c_pop(DisasContext *ctx, arg_c_push *a)
    {
        int reg_list[16] = { 0, 1, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 10, 11 };
        int n_h_size[16] = { 0, 1, 1, 1, 1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4  };
        int sp_offset = (n_h_size[a->c_rcount] + a->c_sp16imm) * 16;
        int r;
        TCGv t0, t1;

        if (!a->c_rcount) {
            return true;
        }

        t0 = tcg_temp_new();
        t1 = tcg_temp_new();

        gen_get_gpr(t0, 2); // t0 = sp
        tcg_gen_addi_tl(t0, t0, sp_offset - a->c_rcount * 4);

        for (r = 1; r <= a->c_rcount; ++r) {
            tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, MO_TESW);
            gen_set_gpr(reg_list[a->c_rcount + 1 - r], t1);
            tcg_gen_addi_tl(t0, t0, 4);
        }

        gen_get_gpr(t0, 2); // t0 = sp
        tcg_gen_addi_tl(t0, t0, sp_offset);
        gen_set_gpr(2, t0);

        tcg_temp_free(t0);
        tcg_temp_free(t1);

        return true;
    }

    static bool trans_c_popret(DisasContext *ctx, arg_c_push *a)
    {
        trans_c_pop(ctx, a);

        exit_tb(ctx); /* no chaining */
        ctx->base.is_jmp = DISAS_NORETURN;

        return true;
    }

    static bool trans_c_push(DisasContext *ctx, arg_c_push *a)
    {
        int reg_list[16] = { 0, 1, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 10, 11 };
        int n_h_size[16] = { 0, 1, 1, 1, 1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4  };
        int sp_offset = (n_h_size[a->c_rcount] + a->c_sp16imm) * 16;
        int r;
        TCGv t0, t1;

        if (!a->c_rcount) {
            return true;
        }

        t0 = tcg_temp_new();
        t1 = tcg_temp_new();

        gen_get_gpr(t0, 2); // t0 = sp
        tcg_gen_addi_tl(t0, t0, 0 - a->c_rcount * 4);

        for (r = 1; r <= a->c_rcount; ++r) {
            gen_get_gpr(t1, reg_list[a->c_rcount + 1 - r]);
            tcg_gen_qemu_st_tl(t1, t0, ctx->mem_idx, MO_TESW);
            tcg_gen_addi_tl(t0, t0, 4);
        }

        gen_get_gpr(t0, 2); // t0 = sp
        tcg_gen_subi_tl(t0, t0, sp_offset);
        gen_set_gpr(2, t0);

        tcg_temp_free(t0);
        tcg_temp_free(t1);

        return true;
    }
    ```
```
# 11. BXXI 参考BXX，参考LD-?LUI
# 12. C.UXTB / C.UXTH 参考ARM UXTB/UXTH
# 13. C.SH / C.LHU 参考RV32I SH/LHU
# 14. MULTADD 参考RV32I MUL/ADD
# 15. J16M / JAL16M 参考 RV32I JMP/JAL
