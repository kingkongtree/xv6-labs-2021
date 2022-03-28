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
- task.json
    ```
        {
            "label": "build",
            "type": "shell",
            "command": "cd build; ../configure --enable-debug --target-list=riscv64-softmmu; make -j8",
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
            "program": "${workspaceFolder}/build/qemu-system-riscv64",
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
    - insn: 根据shfopc不同，同一format可以编码成不同指令；同一arch下opc不能重叠。仅调试的话，改写opc为1111011
        ```
        add_preshf  .. ..... ..... ..... 000 ..... 1111011 @preshf
        sub_preshf  .. ..... ..... ..... 001 ..... 1111011 @preshf
        or_preshf   .. ..... ..... ..... 010 ..... 1111011 @preshf
        xor_preshf  .. ..... ..... ..... 011 ..... 1111011 @preshf
        and_preshf  .. ..... ..... ..... 100 ..... 1111011 @preshf
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
- [Instruction Formats](https://sourceware.org/binutils/docs-2.33.1/as/RISC_002dV_002dFormats.html#RISC_002dV_002dFormats) insn模板貌似仅限于RISCV呢，跟QEMU啥关系？
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
    ```
