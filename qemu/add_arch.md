# 1. [build system](https://qemu.readthedocs.io/en/latest/devel/build-system.html?highlight=add%20new%20arch)
- 1.1 configure：基于autoconf的bash脚本，可以直接CLI执行。
    - 基于meson构建框架，仅支持VPATH模式，意味着需要在build目录作为workdir
    - CLI可以直接识别Meson中的选项配置，‘-'转换为‘_'
- 1.2 meson：用于可执行二进制（保护tool/tests）、文档、ROM和其他二进制文件
    - contrib目录下插件仅支持64bit小端序编译
    - sourcesets: xx_ss代表一个编译目标中的.c文件列表
        - subsystem, emulator/tool下的子系统 ，编译为静态链接库。
        - 全局目标无关：common_ss, softmmu_ss(system mode only), user_ss(user mode only)，需要配合if_false/when字段指明特性相关的差异
        - 全局目标限定：特定CPU/device，指向 hw/ 和 target/ 目录下的子文件夹，关联 configs/target/*.mak，并聚合到target_arch 和 target_softmmu_arch 
        - module：进一步可以分为目标无关或目标相关
        - 公共部分utility：链接到静态库 libqemuutil.a, 包括 util_ss 和 stub_ss
    - *.fa: 指明目标是全链接的，link_whole
    - configs/devices/*.mak: 关联board和device到qemu-system-arch
    - */Kconfig: 定义在具体board/device下，与device/*.mak文件配合使用
    - configs/target/*.mak：生成 *-config-target.h 中的符号，定义TARGET_ARCH
    - build.ninja：meson的生成物，并最终生成GNU Makefile
# 2. [kconfig](https://qemu.readthedocs.io/en/latest/devel/kconfig.html)
- 2.1 QEMU定义了用来描述软件模块间依赖关系的DSL，基于Kconfig（K=kernel, Linux), 对应于 configs目录。
- 2.2 hw/*/Kconfig：其中定义的 CONFIG_* 符号仅在Makefile可见（不可被C代码获取)
    - dependencies: depends on <expr>， 依赖
    - reverse dependencies: select <symbol> [if <expr>]，反向依赖
    - default value: default <value> [if <expr>]，默认值
    - imply <symbol> [if <expr>]，弱反向依赖
# 3. Add TREE Arch
## 3.1 [RISC-V Build Infrastructure](https://gitlab.com/qemu-project/qemu/-/commit/25fa194b7b11901561532e435beb83d046899f7a)
- configure:
    - TARGET_BASE_ARCH/TARGET_ABI_DIR/MTTCG：
        > 新版本在configs/targets/*.mak中定义，无需更改
    - disas_config: 使用riscv-gnu工具链，后面再定义
- defaut-configs/*.mak
    > 新版本在变更目录为/configs/*.mak
- arch_init.c:
    > 文件已从根目录移到 softmmu目录下
- arch_init.h:
- cpus.c
    > 新版本在/meson.build里通过supported_cpus定义, 无需更改
- Makefile.objs
    > 新版本在/target/meson.config中通过subdir定义，无需更改
----
> 以下是具体实现tree指令集
- ./meson.build
    ```
    supported_cpus = [..., 'tree']
    ...
    elif config_host['ARCH'] == 'tree'
        tcg_arch = 'tree'
    ```
- ./configs
    - ./configs/targets/tree-softmmu.mak
        ```
        TARGET_ARCH=tree
        TARGET_BASE_ARCH=tree
        TARGET_SUPPORTS_MTTCG=y
        TARGET_XML_FILES= gdb-xml/riscv-64bit-cpu.xml gdb-xml/riscv-32bit-fpu.xml gdb-xml/riscv-64bit-fpu.xml gdb-xml/riscv-64bit-virtual.xml
        TARGET_NEED_FDT=y
        ```
    - ./configs/devices/tree-softmmu/default.mak
        ```
        CONFIG_RISCV_VIRT=y
        ```
- 。/softmmu/arch_init.c
    ```
    #elif defined(TARGET_TREE)
    #define QEMU_ARCH QEMU_ARCH_TREE
    ```
- ./include/sysemu/arch_init.h
    ```
    QEMU_ARCH_TREE = (1 << 30),
    ```
- ./target
    - ./target/Kconfig
        ```
        source tree/Kconfig
        ```
    - ./target/meson.build
        ```
        subdir('tree')
        ```
    - ./target/tree：整体从riscv复制，并做以下修改
        - TARGET_RISCV64 -> TARGET_TREE
        - ./target/tree/Kconfig
            ```
            config TREE
                bool
            ```
        - ./target/tree/meson.build
            - riscv_ss -> tree_ss
            - riscv_softmmu_ss -> tree_softmmu_ss
            ```
            target_arch += {'tree': tree_ss}
            target_softmmu_arch += {'tree': tree_softmmu_ss}
            ```
        - ./target/tree/cpu.c
            - riscv_cpu_disas_set_info
            > 注释对 print_insn_riscv32 的引用
- ./hw
    - ./hw/Kconfig
        ```
        source tree/Kconfig
        ```
    - ./hw/meson.build
        ```
        subdir('tree')
        ```
    - ./hw/tree
        - ./hw/tree/meson.build
            ```
            tree_ss = ss.source_set()
            hw_arch += {'tree': tree_ss}
            ```
        - ./hw/tree/Kconfig
            > 定义machine等，暂时为空
----
> 编译通过后验证
```
build % ./qemu-system-tree -M help
Supported machines are:
none                 empty machine
```
## 3.2 [RISC-V VirtIO Machine](https://gitlab.com/qemu-project/qemu/-/commit/04331d0b56a0cab2e40a39135a92a15266b37c36)
- TypeInfo *_cpu_type_infos
- *_cpu_class_init
- *_cpu_realize
- *_translate_init
- TranslatorOps *_tr_ops
- hw/*
    - CONFIG_*_DEFAULT_MACHINE
    - meson.build
    - Kconfig
    - *_machine_class_init
    - *_board_init
----
> 实现 tree virtIO machine
- ./hw/tree/Kconfig
    ```
    config RISCV_NUMA
        bool

    config RISCV_VIRT
        bool
        imply PCI_DEVICES
        imply VIRTIO_VGA
        imply TEST_DEVICES
        select RISCV_NUMA
        select GOLDFISH_RTC
        select MSI_NONBROKEN
        select PCI
        select PCI_EXPRESS_GENERIC_BRIDGE
        select PFLASH_CFI01
        select SERIAL
        select SIFIVE_CLINT
        select SIFIVE_PLIC
        select SIFIVE_TEST
        select VIRTIO_MMIO
        select FW_CFG_DMA
    ```
- ./hw/tree/meson.build
    ```
    tree_ss.add(files('boot.c'), fdt)
    tree_ss.add(files('riscv_hart.c'))
    tree_ss.add(when: 'CONFIG_RISCV_NUMA', if_true: files('numa.c'))
    tree_ss.add(when: 'CONFIG_RISCV_VIRT', if_true: files('virt.c'))
    ```
- ./hw/tree/*.c
    - ./hw/riscv/boot.c +-> ./hw/tree/boot.c
    - ./hw/riscv/riscv_hart.c +-> ./hw/tree/riscv_hart.c
    - ./hw/riscv/numa.c +-> ./hw/tree/numa.c
    - ./hw/riscv/virt.c +-> ./hw/tree/virt.c
    ```
    #include "target/tree/cpu.h"
    ```
----
> 验证
```
 build % ./qemu-system-tree -M help                       
Supported machines are:
none                 empty machine
virt                 RISC-V VirtIO board
```
> 仿真运行xv6
```
/Users/kingkongtree/code/simulation/qemu-6.1.0/build/qemu-system-tree -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
```
## 3.3 [RISC-V ELF Machine Definition](https://gitlab.com/qemu-project/qemu/-/commit/f71a8eaffba3271cf7cdad95572f6996f7523a5b)
## 3.4 [RISCV-QEMU v8.2 upstream](https://gitlab.com/qemu-project/qemu/-/commit/d9bbfea646e86426d549bd612cd9f91e49aa50c2)
## 3.5 [Create a SiFive E SoC object](https://gitlab.com/qemu-project/qemu/-/commit/651cd8b7e18eda46a36cf073428452d04bb354f2)
## 3.6 [Activate decodetree and implemnt LUI & AUIPC](https://gitlab.com/qemu-project/qemu/-/commit/2a53cff418335ccb4719e9a94fde55f6ebcc895d)
- 新增 tree.decode
    - ./target/riscv/insn32.decode +-> ./target/tree/tree32.decode
    - ./target/riscv/insn16.decode +-> ./target/tree/tree16.decode
    - ./target/tree/meson.build
        ```
        decodetree.process('tree16.decode', extra_args: ['--static-decode=decode_tree16', '--insnwidth=16']),
        decodetree.process('tree32.decode', extra_args: '--static-decode=decode_tree32'),
        ```
    - ./target/tree/translate.c
        ```
        #include "decode-tree16.c.inc"
        #include "decode-tree32.c.inc"
        #include "insn_trans/trans_tree.c.inc"
        ```
        - decode_opc
            - decode_insn16 -> decode_tree16
            - decode_insn32 -> decode_tree32
    - ./target/tree/insn_trans/trans_tree.c.inc
        > 在这里实现私有指令的trans_**函数
- 新增 trans_tree.c.inc
## 3.7 [Split RVC32 and RVC64 insns into separate files](https://gitlab.com/qemu-project/qemu/-/commit/0e68e240a9bd3b44a91cd6012f0e2bf2a43b9fe2)
## 3.8 [Add a base 32 and 64 bit CPU](https://gitlab.com/qemu-project/qemu/-/commit/8903bf6e6d73d03b988b4a8197132de2ad681ff5)
## 3.9 [Add OpenSBI version 0.4](https://gitlab.com/qemu-project/qemu/-/commit/91f3a2f0ce59cb621630bd224f634955222fc3e0)
## 3.10 [sifive_u: Add support for loading initrd](https://gitlab.com/qemu-project/qemu/-/commit/0f8d4462498afd2f071cb5c837750b703a48ba18)
## 3.11 [Add initial support for Shakti C machine](https://gitlab.com/qemu-project/qemu/-/commit/7a261bafc8ee01294cc709366810798bec4fe2f7)
## 3.12 [riscv: Add documentation for virt machine](https://gitlab.com/qemu-project/qemu/-/commit/85198f189e41c9d9ebe340d2feecf7d668499bc4)