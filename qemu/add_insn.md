# 1. [官方文档](https://qemu.readthedocs.io/en/latest/devel/index.html)
- [Documentation/TCG](https://wiki.qemu.org/Documentation/TCG)
    - [intro README](https://gitlab.com/qemu-project/qemu/-/blob/master/tcg/README) 概述
    - [docs/devel/tcg.rst](https://gitlab.com/qemu-project/qemu/-/blob/master/docs/devel/tcg.rst) 语法、基础ir
    - [decodetree](https://gitlab.com/qemu-project/qemu/-/blob/master/docs/devel/decodetree.rst) 指令格式的抽象描述，pattern
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
----
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
----
````
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
# 6. STMIA
- [STM-STMIA-STMEA--Thumb](https://developer.arm.com/documentation/ddi0406/cb/Application-Level-Architecture/Instruction-Details/Alphabetical-list-of-instructions/STM--STMIA--STMEA-)
- [Arm Armv8-A A32/T32](https://documentation-service.arm.com/static/61c04ba12183326f217711e0?token=) P480
- [CSDN 介绍](https://www.cnblogs.com/lifexy/p/7363208.html)
- ARM-QEMU实现
    - [decodetree-t32](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/t32.decode)
    - [decodetree-a32](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/a32.decode)
    - [translate.c](https://gitlab.com/qemu-project/qemu/-/blob/master/target/arm/translate.c)
----
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
----
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
- nop
```
static bool trans_prf(DisasContext *s, arg_ldst_block *a)
{
    /* prefetch instruction, is a nop instruction in model.  */
    return true;
}
```
# 8. L.LI
- qemu内部的IR本身支持32bit立即数加载，trans函数仿照lui实现即可
```
static bool trans_l.li(DisasContext *ctx, arg_lui *a)
{
    if (a->rd != 0) {
        tcg_gen_movi_tl(cpu_gpr[a->rd], a->imm);
    }
    return true;
}
```
- decodetree.py支持变长指令，如RX指令集支持24bit，使用参数--varinsnwidth定义即可。
```
gen = [
  decodetree.process('insns.decode', extra_args: [ '--varinsnwidth', '32' ])
]
```
# 9. C.LBU/C.SB
- RV32I中对LB和LBU的trans函数实现一致, 同时C.LUI与LUI共用同一个trans函数(gen_load_tl不区分指令长度)
```
static bool gen_load_tl(DisasContext *ctx, arg_lb *a, MemOp memop)
{
    TCGv dest = dest_gpr(ctx, a->rd);
    TCGv addr = get_address(ctx, a->rs1, a->imm);

    tcg_gen_qemu_ld_tl(dest, addr, ctx->mem_idx, memop);
    gen_set_gpr(ctx, a->rd, dest);
    return true;
}

static bool gen_load(DisasContext *ctx, arg_lb *a, MemOp memop)
{
    if (get_xl(ctx) == MXL_RV128) {
        return gen_load_i128(ctx, a, memop);
    } else {
        return gen_load_tl(ctx, a, memop);
    }
}

static bool trans_lb(DisasContext *ctx, arg_lb *a)
{
    return gen_load(ctx, a, MO_SB);
}

static bool trans_lbu(DisasContext *ctx, arg_lbu *a)
{
    return gen_load(ctx, a, MO_UB);
}
```
# 10. C.POP/C.PUSH/C.POPRET
- 参考RX ISA实现
```
static void push(TCGv val)
{
    tcg_gen_subi_i32(cpu_sp, cpu_sp, 4);
    rx_gen_st(MO_32, val, cpu_sp);
}

static void pop(TCGv ret)
{
    rx_gen_ld(MO_32, ret, cpu_sp);
    tcg_gen_addi_i32(cpu_sp, cpu_sp, 4);
}

/* pop rd */
static bool trans_POP(DisasContext *ctx, arg_POP *a)
{
    /* mov.l [r0+], rd */
    arg_MOV_rp mov_a;
    mov_a.rd = 0;
    mov_a.rs = a->rd;
    mov_a.ad = 0;
    mov_a.sz = MO_32;
    trans_MOV_pr(ctx, &mov_a);
    return true;
}

/* popc cr */
static bool trans_POPC(DisasContext *ctx, arg_POPC *a)
{
    TCGv val;
    val = tcg_temp_new();
    pop(val);
    move_to_cr(ctx, val, a->cr);
    if (a->cr == 0 && is_privileged(ctx, 0)) {
        /* PSW.I may be updated here. exit TB. */
        ctx->base.is_jmp = DISAS_UPDATE;
    }
    tcg_temp_free(val);
    return true;
}

/* popm rd-rd2 */
static bool trans_POPM(DisasContext *ctx, arg_POPM *a)
{
    int r;
    if (a->rd == 0 || a->rd >= a->rd2) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "Invalid  register ranges r%d-r%d", a->rd, a->rd2);
    }
    r = a->rd;
    while (r <= a->rd2 && r < 16) {
        pop(cpu_regs[r++]);
    }
    return true;
}


/* push.<bwl> rs */
static bool trans_PUSH_r(DisasContext *ctx, arg_PUSH_r *a)
{
    TCGv val;
    val = tcg_temp_new();
    tcg_gen_mov_i32(val, cpu_regs[a->rs]);
    tcg_gen_subi_i32(cpu_sp, cpu_sp, 4);
    rx_gen_st(a->sz, val, cpu_sp);
    tcg_temp_free(val);
    return true;
}

/* push.<bwl> dsp[rs] */
static bool trans_PUSH_m(DisasContext *ctx, arg_PUSH_m *a)
{
    TCGv mem, val, addr;
    mem = tcg_temp_new();
    val = tcg_temp_new();
    addr = rx_index_addr(ctx, mem, a->ld, a->sz, a->rs);
    rx_gen_ld(a->sz, val, addr);
    tcg_gen_subi_i32(cpu_sp, cpu_sp, 4);
    rx_gen_st(a->sz, val, cpu_sp);
    tcg_temp_free(mem);
    tcg_temp_free(val);
    return true;
}

/* pushc rx */
static bool trans_PUSHC(DisasContext *ctx, arg_PUSHC *a)
{
    TCGv val;
    val = tcg_temp_new();
    move_from_cr(val, a->cr, ctx->pc);
    push(val);
    tcg_temp_free(val);
    return true;
}

/* pushm rs-rs2 */
static bool trans_PUSHM(DisasContext *ctx, arg_PUSHM *a)
{
    int r;

    if (a->rs == 0 || a->rs >= a->rs2) {
        qemu_log_mask(LOG_GUEST_ERROR,
                      "Invalid  register ranges r%d-r%d", a->rs, a->rs2);
    }
    r = a->rs2;
    while (r >= a->rs && r >= 0) {
        push(cpu_regs[r--]);
    }
    return true;
}
```
# 11. BXXI 参考BXX，参考LD-?LUI
# 12. C.UXTB / C.UXTH 参考ARM UXTB/UXTH
# 13. C.SH / C.LHU 参考RV32I SH/LHU
# 14. MULTADD 参考RV32I MUL/ADD
# 15. J16M / JAL16M 参考 RV32I JMP/JAL
