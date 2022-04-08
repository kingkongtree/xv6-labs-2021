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
    // 高8位填充为一个0001=nop, 以规避可能的对齐问题
    __asm__ __volatile__ (".8byte 0x0001deadbeaf049f");
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
# 11. BCONDI
- 实现：对BCOND指令的简单移植
```
# *** bcondi ***
%bi_cmp     24:8
%bi_offset  7:5 20:4

&bi     bi_cmp bi_offset rs1
@bi     ........ .... ..... ... ..... ....... &bi %bi_cmp %bi_offset %rs1

# change opc from 0111011 to 0111111 for overlap
beqi    ........ .... ..... 000 ..... 0111111 @bi
bnei    ........ .... ..... 001 ..... 0111111 @bi
blti    ........ .... ..... 100 ..... 0111111 @bi
bgei    ........ .... ..... 101 ..... 0111111 @bi
bltui   ........ .... ..... 110 ..... 0111111 @bi
bgeui   ........ .... ..... 111 ..... 0111111 @bi


// porting from gen_branch
static bool gen_branchi(DisasContext *ctx, arg_bi *a, TCGCond cond)
{
    TCGLabel *l = gen_new_label();
    TCGv source1, source2;
    source1 = tcg_temp_new();
    source2 = tcg_temp_new();
    gen_get_gpr(source1, a->rs1);
    tcg_gen_movi_tl(source2, a->bi_cmp);

    tcg_gen_brcond_tl(cond, source1, source2, l);
    gen_goto_tb(ctx, 1, ctx->pc_succ_insn);
    gen_set_label(l); /* branch taken */

    if (!has_ext(ctx, RVC) && ((ctx->base.pc_next + (a->bi_offset << 1)) & 0x3)) { // bi_offset align to 16bytes
        /* misaligned */
        gen_exception_inst_addr_mis(ctx);
    } else {
        gen_goto_tb(ctx, 0, ctx->base.pc_next + a->bi_offset);
    }
    ctx->base.is_jmp = DISAS_NORETURN;

    tcg_temp_free(source1);
    tcg_temp_free(source2);

    return true;
}

static bool trans_beqi(DisasContext *ctx, arg_beqi *a)
{
    return gen_branchi(ctx, a, TCG_COND_EQ);
}

static bool trans_bnei(DisasContext *ctx, arg_bnei *a)
{
    return gen_branchi(ctx, a, TCG_COND_NE);
}

static bool trans_blti(DisasContext *ctx, arg_blti *a)
{
    return gen_branchi(ctx, a, TCG_COND_LT);
}

static bool trans_bgei(DisasContext *ctx, arg_bgei *a)
{
    return gen_branchi(ctx, a, TCG_COND_GE);
}

static bool trans_bltui(DisasContext *ctx, arg_bltui *a)
{
    return gen_branchi(ctx, a, TCG_COND_LTU);
}

static bool trans_bgeui(DisasContext *ctx, arg_bgeui *a)
{
    return gen_branchi(ctx, a, TCG_COND_GEU);
}
```
- 验证
```
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
```
# 12. C.UXTB / C.UXTH 参考ARM UXTB/UXTH
```
# *** c.utxb / c.utxh ***
&c_utx      rs1    !extern
@c_utx      ...... ... .. .....  &c_utx  rs1=%rs1_3

# b5:2 from 00/01 to 10/11 for overlap
c_utxb      100111 ... 10 00001  @c_utx
c_utxh      100111 ... 11 00001  @c_utx

static bool trans_c_utxb(DisasContext *ctx, arg_c_utx *a)
{
    TCGv t_rs = tcg_temp_new();
    gen_get_gpr(t_rs, a->rs1);

    tcg_gen_andi_tl(t_rs, t_rs, 255);
    gen_set_gpr(a->rs1, t_rs);

    tcg_temp_free(t_rs);

    return true;
}

static bool trans_c_utxh(DisasContext *ctx, arg_c_utx *a)
{
    TCGv source1 = tcg_temp_new();
    TCGv source2 = tcg_temp_new();

    gen_get_gpr(source1, a->rs1);

    tcg_gen_movi_tl(source2, 16);
    tcg_gen_shl_tl(source1, source1, source2);
    tcg_gen_shr_tl(source1, source1, source2);

    gen_set_gpr(a->rs1, source1);
    tcg_temp_free(source1);
    tcg_temp_free(source2);
    return true;
}
```
# 13. C.SH / C.LHU 参考RV32I SH/LHU
```
# *** c.sh / c.lhu ***
# b0:2 from 10 to 11 for overlap
c_lhu       001  ... ... .. ... 11 @cl_tree
c_sh        101  ... ... .. ... 11 @cs_tree

static bool trans_c_lhu(DisasContext *ctx, arg_i *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm << 1); // step 16-bits

    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, MO_UW);
    gen_set_gpr(a->rd, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_sh(DisasContext *ctx, arg_s *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm << 1); // step 16-bits
    gen_get_gpr(dat, a->rs2);

    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, MO_SW);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}
```
# 14. MULTADD 参考RV32I MUL/ADD
```
# *** muliadd ***
%muli_uimm 14:1 25:7
&muliadd  uimm rs2 rs1 rd

@muliadd  ....... ..... ..... . .. ..... ....... &muliadd  uimm=%muli_uimm %rs2 %rs1 %rd

muliadd   ....... ..... ..... . 01 ..... 1011011 @muliadd

static bool trans_muliadd(DisasContext *ctx, arg_muliadd *a)
{
    TCGv tmp_rs1 = tcg_temp_new();
    TCGv tmp_rs2 = tcg_temp_new();

    gen_get_gpr(tmp_rs1, a->rs1);
    gen_get_gpr(tmp_rs2, a->rs2);
    tcg_gen_muli_tl(tmp_rs2, tmp_rs2, a->uimm);
    tcg_gen_add_tl(tmp_rs1, tmp_rs1, tmp_rs2);

    gen_set_gpr(a->rd, tmp_rs1);

    tcg_temp_free(tmp_rs1);
    tcg_temp_free(tmp_rs2);

    return true;
}

```
# 15. J16M / JAL16M 参考 RV32I JMP/JAL
```
# *** j16m / jal16m ***
%j16m   21:s10 20:1 12:8 31:1 8:4
&j16m   simm

@j16m   . .......... . ........ .... . .......  &j16m simm=%j16m

j16m    . .......... . ........ .... 1 1111011  @j16m
jal16m  . .......... . ........ .... 0 1111011  @j16m

static bool trans_j16m(DisasContext *ctx, arg_j16m *a)
{
    gen_goto_tb(ctx, 0, a->simm << 1); // pc = sext(simm * 2)
    return true;
}

static bool trans_jal16m(DisasContext *ctx, arg_j16m *a)
{
    gen_jal(ctx, 1, a->simm << 1); // pc = ra + sext(simm * 2)
    return true;
}
```
# 16. debug stmia/ldmia
## 16.1 Illegal Instruction
- 验证stmia时，xv6在qemu中报错
```
usertrap(): unexpected scause 0x0000000000000002 pid=7
            sepc=0x0000000000000284 stval=0x0000000000000000
> scause=2="Illegal instruction", 指示S-mode时的trap原因
> sepc=0x284，指示的是触发trap的指令0xff11f0b的虚拟地址
> 查看并gdb添加断点调试xv6代码，发现scause=2不是kernel的行为，应该是qemu模拟的硬件行为。
```
- guest os user program
```
INSN(0x07f11f0b)
```
- gdb 添加断点后找到对应qemu的调用栈
```
void riscv_cpu_do_interrupt(CPUState *cs)
{
    bool async = !!(cs->exception_index & RISCV_EXCP_INT_FLAG); // false
    target_ulong cause = cs->exception_index & RISCV_EXCP_INT_MASK; // 2
    
    if (env->priv <= PRV_S &&
            cause < TARGET_LONG_BITS && ((deleg >> cause) & 1)) {
    } else {
        env->mcause = cause | ~(((target_ulong)-1) >> async); // 2
        env->mepc = env->pc; // 0x284
    }
}

riscv_cpu_do_interrupt (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/cpu_helper.c:1053)
cpu_handle_exception (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:663)
cpu_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:913)
tcg_cpus_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops.c:67)
mttcg_cpu_thread_fn (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops-mttcg.c:70)
qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
_pthread_start (@_pthread_start:40)
```
- 定位exception_index的产生原因
```
static void decode_opc(CPURISCVState *env, DisasContext *ctx, uint16_t opcode)
{
    if (extract16(opcode, 0, 5) == 0x1f) { // 48-bits with opcode[0:5] == 11111
    } else if (extract16(opcode, 0, 2) != 3) { // 16-bits with opcode[0:2] in {00, 01, 02}
    } else { // 32-bits with opcode[0:2] == 11; 64-bits with opcode[0:6] == 111111
        uint32_t opcode32 = opcode;
        opcode32 = deposit32(opcode32, 16, 16,
                             translator_lduw(env, ctx->base.pc_next + 2));
        ctx->pc_succ_insn = ctx->base.pc_next + 4;
        if (!decode_tree32(ctx, opcode32)) {
            gen_exception_illegal(ctx);
        }
    }
}

gen_exception_illegal (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:163)
decode_opc (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:948)
riscv_tr_translate_insn (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:1003)
translator_loop (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/translator.c:93)
gen_intermediate_code (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:1058)
tb_gen_code (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/translate-all.c:1470)
cpu_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:945)
tcg_cpus_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops.c:67)
mttcg_cpu_thread_fn (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops-mttcg.c:70)
qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
_pthread_start (@_pthread_start:40)
```
> 原因是decode_tree32解析指令不成功, trans_stmia返回了false
- host qemu trans
```
static bool trans_stmia(DisasContext *s, arg_stmia *a)
{
    int callee_reg_list[16] = { 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 9, 8, 2, -1, -1, -1 };
    int caller_reg_list[16] = { 31, 30, 29, 28,  17, 16, 15, 14, 13, 12, 11, 10, 7, 6, 5, 1 };
    int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
    unsigned int mask = a->gpr_mask;

    //for (int i = 0; i < 16; ++i) {
    //    if (a->rs1 == reg_list[i]) {
    //        return false;
    //    }
    //}

    for (int i = 0; i < 16; ++i) {
        if (reg_list[i] == -1) break; // if meet undef reg, ignore all left.
        if (!(mask & (1 << i))) continue; // check if masked

        if (a->rs1 == reg_list[i]) { // rs1 not should masked
            return false;
        }
    }

    return true;
}
```
> 对 0x0ff11f0b 解码，a->rs1=x2在callee_reg_list中； 此处判断返回false逻辑移到循环内部。
- in_asm.log
```
0000000000000278 <main>:

int main(int argc, char *argv[])
{
 282:	1000                	addi	s0,sp,32
 284:	0ff11f0b          	0xff11f0b
    INSN(0x0ff11f0b)

    if (argc > 1) {
 288:	4785                	li	a5,1
```
- out_asm.log
```
  -- guest addr 0x0000000000000282
  1000                	addi	s0,sp,32
0x2800ac4d4:  91008294  add      x20, x20, #0x20
0x2800ac4d8:  f9002274  str      x20, [x19, #0x40]
  -- guest addr 0x0000000000000284
  0ff11f0b          	0xff11f0b
0x2800ac4dc:  52805094  mov      w20, #0x284
0x2800ac4e0:  f9031674  str      x20, [x19, #0x628]
0x2800ac4e4:  aa1303e0  mov      x0, x19
0x2800ac4e8:  52800041  mov      w1, #2
0x2800ac4ec:  580004be  ldr      x30, #0x2800ac580
0x2800ac4f0:  d63f03c0  blr      x30
0x2800ac4f4:  70fff060  adr      x0, #0x2800ac303
0x2800ac4f8:  17fd4ecd  b        #0x28000002c
  -- tb slow paths + alignment
  -- guest addr 0x0000000080001cd6 + tb prologue
  c501                	beqz	a0,80001cd6 <usertrap+0xa4>
0x2800ac700:  b85f0274  ldur     w20, [x19, #-0x10]
```
- 验证, ‘Illegal instruction’错误消失，scause变成了0xf=‘Store/AMO page fault’
## 16.2 Store page fault
```
usertrap(): unexpected scause 0x000000000000000f pid=3
            sepc=0x0000000000000284 stval=0x0000000000003000
```
> sepc=0x284指向同一条指令，但scause=0xf=“Store page fault“，stval=0x3000指向虚拟地址
```
If stval is written with a nonzero value when a misaligned load or store causes an access-fault or page-fault exception, then stval will contain the virtual address of the portion of the access that caused the fault.
```
> gdb debug到写异常的点
```
// addr = 0x3000，按照xv6的39bit va + 3级pte的方式，最低一级pte为0，非法。
static int get_physical_address(CPURISCVState *env, hwaddr *physical,
                                int *prot, target_ulong addr,
                                target_ulong *fault_pte_addr,
                                int access_type, int mmu_idx,
                                bool first_stage, bool two_stage,
                                bool is_debug)
{
    mode = mmu_idx & TB_FLAGS_PRIV_MMU_MASK; // 0
    mxr = get_field(env->mstatus, MSTATUS_MXR); // 0
    base = (hwaddr)get_field(env->satp, SATP64_PPN) << PGSHIFT; // 0x87f73000
    vm = get_field(env->satp, SATP64_MODE); // 8
    sum = get_field(env->mstatus, MSTATUS_SUM) || use_background || is_debug; // 0
    switch (vm) {
    case VM_1_10_SV39:
      levels = 3; ptidxbits = 9; ptesize = 8; break;
    }
    va_bits = PGSHIFT + levels * ptidxbits + widened; // 39
    mask = (1L << (TARGET_LONG_BITS - (va_bits - 1))) - 1; // 0x3ffffff
    masked_msbs = (addr >> (va_bits - 1)) & mask; // 0
    int ptshift = (levels - 1) * ptidxbits; // 18

    for (i = 0; i < levels; i++, ptshift -= ptidxbits) { // i = 2, ptshift = 0
        idx = (addr >> (PGSHIFT + ptshift)) & ((1 << ptidxbits) - 1); // 3
        pte_addr = base + idx * ptesize; // 0x87f65018
        pte = address_space_ldq(cs->as, pte_addr, attrs, &res); // 0

        if (!(pte & PTE_V)) {
            /* Invalid PTE */
            return TRANSLATE_FAIL;
        }
    }
}

bool riscv_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                        MMUAccessType access_type, int mmu_idx,
                        bool probe, uintptr_t retaddr)
{
    if (riscv_cpu_virt_enabled(env) ||
        ((riscv_cpu_two_stage_lookup(mmu_idx) || two_stage_lookup) &&
         access_type != MMU_INST_FETCH)) {
        /* Two stage lookup */
        ret = get_physical_address(env, &pa, &prot, address,
                                   &env->guest_phys_fault_addr, access_type,
                                   mmu_idx, true, true, false);
    }

    f (ret == TRANSLATE_SUCCESS) {
    } else if (probe) {
    } else {
        raise_mmu_exception(env, address, access_type, pmp_violation,
                            first_stage_error,
                            riscv_cpu_virt_enabled(env) ||
                                riscv_cpu_two_stage_lookup(mmu_idx));
        riscv_raise_exception(env, cs->exception_index, retaddr);
    }
}

raise_mmu_exception (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/cpu_helper.c:671)
riscv_cpu_tlb_fill (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/cpu_helper.c:887)
tlb_fill (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cputlb.c:1304)
store_helper (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cputlb.c:2432)
addr = 0x3000, val = 99 ?
helper_le_stq_mmu (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cputlb.c:2541)
280108650 (@280108650..2801086d0:3)
cpu_tb_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:353)
tb->pc = 0x278
cpu_loop_exec_tb (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:812)
cpu_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:970)
tcg_cpus_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops.c:67)
mttcg_cpu_thread_fn (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops-mttcg.c:70)
qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
_pthread_start (@_pthread_start:40)
```
- 先调试load，排除寄存器冲突, 从x27=s11开始，依次减少stm的寄存器，发现在对x9=s1 load后出现异常
> x2=sp, x8=s0在smp时复用为fp，那么x9=s1是第一个callee save寄存器，从uptime.asm文件中发现s1在main里面被作为a0的临时寄存器使用
## 16.3 修复用例
```
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

    printf(" ==>ldmia {x27} (sp)\n");
    print_gprs();
    INSN(0x0001008b) // x27 = s11

    printf(" ==>ldmia {x23-x27} (sp)\n");
    print_gprs();
    INSN(0x00010f8b) // x23-x27 = s7-s11

    printf(" ==>ldmia {x19-x27} (sp)\n");
    print_gprs();
    INSN(0x00f10f8b) // x19-x27 = s3-s11

    printf(" ==>ldmia {x18-27} (sp)\n");
    print_gprs();
    INSN(0x01f10f8b) // x18-x27 =s2-s11

    printf(" ==>stmia {x27} (sp)\n");
    print_gprs();
    INSN(0x0001108b) // x27 = s11

    printf(" ==>stmia {x23-x27} (sp)\n");
    print_gprs();
    INSN(0x00011f8b) // x23-x27 = s7-s11

    printf(" ==>stmia {x19-27} (sp)\n");
    print_gprs();
    INSN(0x00f11f8b) // x19-x27 = s3-s11

    printf(" ==>stmia {x18-27} (sp)\n");
    print_gprs();
    INSN(0x01f11f8b) // x18-x27 =s2-s11

    // printf(" ==>stmia {x9, x18-27} (sp)\n");
    // print_gprs();
    // INSN(0x03f12f8b) // x9, x18-x27 = s1, s2-s11

    // printf(" ==>stmia {x8-x9, x18-27} (sp)\n");
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

    printf(" ==>ldmia {x27} (sp)\n");
    print_gprs();
    INSN(0x0001008b) // x27 = s11

    printf(" ==>ldmia {x23-x27} (sp)\n");
    print_gprs();
    INSN(0x00010f8b) // x23-x27 = s7-s11

    printf(" ==>ldmia {x19-x27} (sp)\n");
    print_gprs();
    INSN(0x00f10f8b) // x19-x27 = s3-s11

    printf(" ==>ldmia {x18-27} (sp)\n");
    print_gprs();
    INSN(0x01f10f8b) // x18-x27 =s2-s11

    // printf(" ==>ldmia {x9, x18-27} (sp)\n");
    // print_gprs();
    // INSN(0x03f10f8b) // x9, x18-x27 = s1, s2-s11

    // printf(" ==>ldmia {x8-x9, x18-x27} (sp)\n");
    // print_gprs();
    // INSN(0x07f10f8b) // x8-x9, x18-x27 = s0-s1, s2-s11
}
```
- 验证通过
```
================ mia test ===========
 - x2=sp= 0x2FB0
 - x1=ra= 0x3F8
 - x8=fp= 0x2FC0
 ==>ldmia {x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
 ==>ldmia {x23-x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 12, 13, 14, 15, 16, 17, 18, 19, 20, 12256
 ==>ldmia {x19-x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 12, 13, 14, 15, 16, 16288, 81744, 99, 1016, 12256
 ==>ldmia {x18-27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 12, 1769238645, 0, 12272, 254, 16288, 81744, 99, 1016, 12256
 ==>stmia {x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 0, 1769238645, 0, 12272, 254, 16288, 81744, 99, 1016, 12256
 ==>stmia {x23-x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 0, 1769238645, 0, 12272, 254, 16288, 81744, 99, 1016, 12256
 ==>stmia {x19-27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 0, 1769238645, 0, 12272, 254, 16288, 81744, 99, 1016, 12256
 ==>stmia {x18-27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 11
 - x18:x27=s2:11= 0, 1769238645, 0, 12272, 254, 16288, 81744, 99, 1016, 12256
 ==>ldmia {x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 101
 - x18:x27=s2:11= 102, 103, 104, 105, 106, 107, 108, 109, 110, 111
 ==>ldmia {x23-x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 101
 - x18:x27=s2:11= 102, 103, 104, 105, 106, 107, 108, 109, 110, 12256
 ==>ldmia {x19-x27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 101
 - x18:x27=s2:11= 102, 103, 104, 105, 106, 16288, 81744, 99, 1016, 12256
 ==>ldmia {x18-27} (sp)
 - x2=sp= 0x2F10
 - x8=fp= 0x2FB0
 - x9=s1= 101
 - x18:x27=s2:11= 102, 1769238645, 0, 12272, 254, 16288, 81744, 99, 1016, 12256
```
# 17. l.li debug
## 17.1 illegal instruction usertrap
```
usertrap(): unexpected scause 0x0000000000000002 pid=3
            sepc=0x000000000000040b stval=0x0000000000000000

 3f8:	00001517          	auipc	a0,0x1
 3fc:	ab850513          	addi	a0,a0,-1352 # eb0 <malloc+0x3aa>
 400:	00000097          	auipc	ra,0x0
 404:	648080e7          	jalr	1608(ra) # a48 <printf>
    test_l_li();
 408:	049f beaf dead      	0xdeadbeaf049f
 40e:	0001                	nop
```
- 0x40b的指令地址，符合对 decode_opc 的修改
```
static void decode_opc(CPURISCVState *env, DisasContext *ctx, uint16_t opcode)
{
    /* check for 48-bits private insn */
    if (extract16(opcode, 0, 5) == 0x1f) { // 48-bits with opcode[0:5] == 11111
        uint64_t opcode48 = opcode;
        opcode48 = deposit64(opcode48, 16, 32, // 16 + 32 = 48
                             translator_ldl(env, ctx->base.pc_next + 2));
        if (!decode_tree48(ctx, opcode48)) {
            gen_exception_illegal(ctx);
        }
        ctx->pc_succ_insn = ctx->base.pc_next + 3; // 0x40b = 0x408 + 3, 即高8bit的指令
    /* check for compressed insn */
    } else if (extract16(opcode, 0, 2) != 3) { // 16-bits with opcode[0:2] in {00, 01, 02}
        if (!has_ext(ctx, RVC)) {
            gen_exception_illegal(ctx);
        } else {
            ctx->pc_succ_insn = ctx->base.pc_next + 2; // 0x40b 进入此分支
        #if defined(TARGET_TREE)
            if (!decode_tree16(ctx, opcode)) {
        #else
            if (!decode_insn16(ctx, opcode)) {
        #endif
                gen_exception_illegal(ctx); // 提前修改了 pc_succ_insn = 0x40d
            }
        }
    }
}

decode_opc (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:923)
riscv_tr_translate_insn (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:1003)
translator_loop (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/translator.c:93)
gen_intermediate_code (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/tree/translate.c:1058)
tb_gen_code (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/translate-all.c:1470)
cpu_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/cpu-exec.c:945)
tcg_cpus_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops.c:67)
mttcg_cpu_thread_fn (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops-mttcg.c:70)
qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
_pthread_start (@_pthread_start:40)
```
> 发现一个实现的问题，decode_tree16 是先计算 pc_succ_insn，而 decode_tree48 写成了后计算。修改后发现未解决问题。
> 在上报illegal insn exception的地方断点，发现64bit指令中第二次解析出来的opcode=0xadbe, 比预期中地址少加了3。pc_succ_insn = pc_next + 6, 而非 + 3.
```
    ctx->pc_succ_insn = ctx->base.pc_next + 6; // increase pc first, step 8bits
```
- 验证
```
void test_l_li(void)
{
    register int x2 asm("sp");
    register int x1 asm("ra");
    register int x8 asm("s0");
    printf(" - x2=sp= 0x%x\n - x1=ra= 0x%x\n - x8=fp= 0x%x\n", x2, x1, x8);

    register int x9 asm("s1");

    printf(" - x9=s1= 0x%x\n", x9);

    // 高8位填充为一个16bit指令, 852a=mov a0,a0, 以规避可能的对齐问题
    printf(" ====> li s1,#0xdeadbeaf\n");
    __asm__ __volatile__ (".8byte 0x852adeadbeaf049f");

    printf(" - x9=s1= 0x%x\n", x9);
}

================ l.li test ===========
 - x2=sp= 0x3FA0
 - x1=ra= 0x46A
 - x8=fp= 0x3FC0
 
 - x9=s1= 0x65
 ====> li s1,#0xdeadbeaf
 - x9=s1= 0xDEADBEAF
```