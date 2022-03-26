# 1. vscode launch.json
    ```
    "version": "0.2.0",
    "configurations": [
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
                // "-plugin", "./contrib/plugins/libexeclog.so",
                "-d", "in_asm", "-D", "in_asm.log",
                "-d", "op", "-D", "op.log"
            ],
            "cwd": "${workspaceFolder}",
            // "preLaunchTask": "build"
        },
    ]
    ```
# 1. qemu_init
    ```
    qemu_init (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/vl.c:3638)
    qemu_main (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/main.c:49)
    call_qemu_main (/Users/kingkongtree/code/simulation/qemu-6.1.0/ui/cocoa.m:1885)
    qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
    _pthread_start (@_pthread_start:40)
    ```
## 1.1 qemu_init_main_loop
## 1.2 qemu_create_machine
- machine_class
    - name: "virt"
    - desc: "RISC-V VirtIO board"
    - default_cpu_type: "rv64-riscv-cpu"
    - default_cpus: 1
    - smp_parse: (qemu-system-riscv64`smp_parse at machine.c:747)
    - init: (qemu-system-riscv64`virt_machine_init at virt.c:545)
    - minimum_page_bits: 0
    - default_ram_size: 134217728
## 1.3 qemu_create_default_devices
- add_device_config(DEV_PARALLEL, "null")
- add_device_config(DEV_SERIAL, "mon:stdio")
## 1.4 qemu_create_early_backends
- qemu_display_early_init
- qemu_console_early_init
- configure_blockdev
- audio_init_audiodevs
## 1.5 qemu_create_late_backends
- net_init_clients
- foreach_device_config(DEV_SERIAL, serial_parse)
- foreach_device_config(DEV_PARALLEL, parallel_parse)
- foreach_device_config(DEV_DEBUGCON, debugcon_parse)
- qemu_semihosting_console_init();
## 1.6 qemu_resolve_machine_memdev()
## 1.7 qmp_x_exit_preconfig
### 1.7.1 qemu_init_board
#### 1.7.1.1 machine_run_board_init
- machine.kernel_filename: "../xv6-labs-2021-my/kernel/kernel"
##### 1.7.1.1.2 virt_machine_init
- RISCVVirtState
    ```
    struct RISCVCPU {
        ...
        CPURISCVState env; // 最关键的定义，CPU硬件上下文
        ...
    };
    struct RISCVHartArrayState {
        ...
        RISCVCPU *harts; // RISC-V的hart，通常代表一个线程资源和一个物理Core
        ...
    };
    struct RISCVVirtState {
        /*< private >*/
        MachineState parent; // virt machine 挂接的上级设备

        /*< public >*/
        RISCVHartArrayState soc[VIRT_SOCKETS_MAX]; // Core 硬件上下文
        DeviceState *plic[VIRT_SOCKETS_MAX];
        PFlashCFI01 *flash[2];
        FWCfgState *fw_cfg;

        int fdt_size;
    };
    ```
- init soc: hart,bus,CLINT,PLIC
- RAM,Device tree,ROM,BIOS // 参数未指明BIOS，因此使用QEMU默认从0x80000000开始的加载方式
- riscv_load_kernel(machine->kernel_filename, kernel_start_addr, NULL);
    - kernel_start_addr: 2147483648 = 0x80000000, 即对应xv6的`0x80000000 <_entry>:`
    - load_elf_ram_sym -> load_elf32 -> glue
    - kernel_entry = 0x80000000
- riscv_setup_rom_reset_vec // 将reset向量也指向0x80000000
- VirtIO,IRQ,PCIE,UART,Flash
### 1.7.2 qemu_create_cli_devices
- device_config(DEV_USB, usb_parse)
## 1.8 qemu_init_displays
- qemu_display_init
## 1.9 os_setup_post
- change_root();
- change_process_uid();
# 2. qemu_main_loop
    ```
    qemu_main_loop (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/runstate.c:726)
    qemu_main (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/main.c:50)
    call_qemu_main (/Users/kingkongtree/code/simulation/qemu-6.1.0/ui/cocoa.m:1885)
    qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
    _pthread_start (@_pthread_start:40)
    ```
## 2.1