# 1. 官方文档
- [Documentation/TCG](https://wiki.qemu.org/Documentation/TCG)
    - [intro README](https://gitlab.com/qemu-project/qemu/-/blob/master/tcg/README) 概述
    - [docs/devel/tcg.rst](https://gitlab.com/qemu-project/qemu/-/blob/master/docs/devel/tcg.rst) 语法、基础ir
    - [decodetree](https://gitlab.com/qemu-project/qemu/-/blob/master/docs/devel/decodetree.rst) 指令格式的抽象描述，pattern
    - [backend ops](https://wiki.qemu.org/Documentation/TCG/backend-ops) qemu官方定义的原子指令，后端表示即IR
    - [frontend ops](https://wiki.qemu.org/Documentation/TCG/frontend-ops) 后端op的C表示
- [riscv-isa-manual](https://github.com/riscv/riscv-isa-manual)
    - [RISC-V Privileged ISA v1.12](https://github.com/riscv/riscv-isa-manual/releases/download/Priv-v1.12/riscv-privileged-20211203.pdf)
    - [RISC-V Unprivileged ISA v2.1](https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf)
- [ARM](https://documentation-service.arm.com/)
    - [Arm Armv9-A A64](https://documentation-service.arm.com/static/61c04cbab691546d37bd2b87?token=)
    - [Arm Armv8-A A32/T32](https://documentation-service.arm.com/static/61c04ba12183326f217711e0?token=)
# 2. 参考文档
- 教程
    - [riscv gcc中添加custom自定义指令](https://cloud.tencent.com/developer/article/1886469)
    - [riscv实现自定义指令并用qemu运行](https://cloud.tencent.com/developer/article/1819855?from=article.detail.1770528)
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
    - insn: 根据shfopc不同，同一format可以编码成不同指令
        ```
        preshf_add  .. ..... ..... ..... 000 ..... 0011011 @preshf
        preshf_sub  .. ..... ..... ..... 001 ..... 0011011 @preshf
        preshf_or   .. ..... ..... ..... 010 ..... 0011011 @preshf
        preshf_xor  .. ..... ..... ..... 011 ..... 0011011 @preshf
        preshf_and  .. ..... ..... ..... 100 ..... 0011011 @preshf
        ```
    - .decode: 定义一个新的独立的.decode文件
        - decodetree.py: python实现的对insn pattern->c的翻译
            - to be implemented
        - .meson: python的配置文件
            - to be implemented
- 前端实现：c translate
    - trans_xxx: 定义宏GEN_TRANS_PRESHF，对shfopc抽象处理
        ```
        #define GEN_TRANS_PRESHF(SHFTYPE)                                                   \
        static bool trans_preshf_##SHFTYPE##(DisasContext *ctx, arg_preshf_##SHFTYPE## *a)  \
        {                                                                                   \
            return gen_preshf(ctx, a, gen_preshf_##SHFTYPE##);                              \
        }                                                                                   \

        GEN_TRANS_PRESHF(add)
        GEN_TRANS_PRESHF(sub)
        GEN_TRANS_PRESHF(or)
        GEN_TRANS_PRESHF(xor)
        GEN_TRANS_PRESHF(and)
        ```
    - gen_preshf: 按shftype
        ```
        static bool gen_preshf(DisasContext *ctx, arg_r *a, void(*func)(TCGv, TCGv, TCGv))
        {
            TCGv dst = tcg_temp_new();
            TCGv src = tcg_temp_new();
            TCGv shfdst = tcg_temp_new();
            TCGv shfamt = tcg_temp_new();
            TCGv shfsrc = tcg_temp_new();

            gen_get_gpr(shfsrc, a->rs2);
            gen_get_gpr(shfamt, a->shfamt);

            switch(a->shftype) { // by shftype
                case 0: // 00
                    gen_presll(shfdst, shfsrc, shfamt);
                    break;
                case 1: // 01
                    gen_presrl(shfdst, shfsrc, shfamt);
                    break;
                case 2; // 10
                    gen_presra(shfdst, shfsrc, shfamt);
                    break;
                case 3: // 11
                    gen_preror(shfdst, shfsrc, shfamt);
                    break;
                default:
                    return false;
            }

            gen_get_gpr(dst, a->rd);
            gen_get_gpr(src, a->rs1);

            (*func)(dst, src, shfdst); // by shfopc

            gen_set_gpr(a->rd, dst);

            tcg_temp_free(shfdst);
            tcg_temp_free(shfsrc);
            tcg_temp_free(shfamt);
            tcg_temp_free(src);
            tcg_temp_free(dst);

            return true;
        }
        ```
    - gen_prexxx: preshf原子定义, 基本上是对qemu原生op的直接映射
        - by shftype
            ```
            static void gen_presll(TCGv dst, TCGv src, TCGv amt)
            {
                tcg_gen_shl_tl(dst, src, amt);
            }
            static void gen_presrl(TCGv dst, TCGv src, TCGv amt)
            {
                tcg_gen_shr_tl(dst, src, amt);
            }
            static void gen_presra(TCGv dst, TCGv src, TCGv amt)
            {
                tcg_gen_sar_tl(dst, src, amt);
            }
            static void gen_preror(TCGv dst, TCGv src, TCGv amt)
            {
                tcg_gen_rotr_tl(dst, src, amt);
            }
            ```
        - by shfopc:
            ```
            static void gen_preshf_add(TCGv dst, TCGv src1, TCGv src2)
            {
                tcg_gen_add_tl(dst, src1, src2);
            }
            static void gen_preshf_sub(TCGv dst, TCGv src1, TCGv src2)
            {
                tcg_gen_sub_tl(dst, src1, src2);
            }
            static void gen_preshf_add(TCGv dst, TCGv src1, TCGv src2)
            {
                tcg_gen_or_tl(dst, src1, src2);
            }
            static void gen_preshf_add(TCGv dst, TCGv src1, TCGv src2)
            {
                tcg_gen_xor_tl(dst, src1, src2);
            }
            static void gen_preshf_and(TCGv dst, TCGv src1, TCGv src2)
            {
                tcg_gen_and_tl(dst, src1, src2);
            }
            ```