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
# 2. qemu_init_vcpu
- backtrace: qemu_init_board->virt_machine_init->sysbus_realize->riscv_hart_realize->riscv_cpu_realize
    ```
    __psynch_cvwait (@__psynch_cvwait:5)
    _pthread_cond_wait (@_pthread_cond_wait:312)
    qemu_cond_wait_impl (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:194)
    qemu_init_vcpu (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/cpus.c:633)
    riscv_cpu_realize (/Users/kingkongtree/code/simulation/qemu-6.1.0/target/riscv/cpu.c:562)
    device_set_realized (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/core/qdev.c:761)
    property_set_bool (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/object.c:2258)
    object_property_set (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/object.c:1403)
    object_property_set_qobject (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/qom-qobject.c:28)
    object_property_set_bool (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/object.c:1473)
    qdev_realize (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/core/qdev.c:389)
    riscv_hart_realize (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/riscv/riscv_hart.c:52)
    riscv_harts_realize (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/riscv/riscv_hart.c:63)
    device_set_realized (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/core/qdev.c:761)
    property_set_bool (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/object.c:2258)
    object_property_set (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/object.c:1403)
    object_property_set_qobject (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/qom-qobject.c:28)
    object_property_set_bool (/Users/kingkongtree/code/simulation/qemu-6.1.0/qom/object.c:1473)
    qdev_realize (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/core/qdev.c:389)
    sysbus_realize (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/core/sysbus.c:256)
    virt_machine_init (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/riscv/virt.c:597)
    machine_run_board_init (/Users/kingkongtree/code/simulation/qemu-6.1.0/hw/core/machine.c:1273)
    qemu_init_board (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/vl.c:2618)
    qmp_x_exit_preconfig (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/vl.c:2692)
    qemu_init (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/vl.c:3713)
    qemu_main (/Users/kingkongtree/code/simulation/qemu-6.1.0/softmmu/main.c:49)
    call_qemu_main (/Users/kingkongtree/code/simulation/qemu-6.1.0/ui/cocoa.m:1885)
    qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
    _pthread_start (@_pthread_start:40)
    ```
- qemu_cond_wait，完成soc的初始化后，vcpu立即启动
    ```
    void qemu_init_vcpu(CPUState *cpu)
    {
        MachineState *ms = MACHINE(qdev_get_machine());

        cpu->nr_cores = ms->smp.cores;
        cpu->nr_threads =  ms->smp.threads;
        cpu->stopped = true;

        if (!cpu->as) {
            cpu_address_space_init(cpu, 0, "cpu-memory", cpu->memory);
        }

        cpus_accel->create_vcpu_thread(cpu);

        while (!cpu->created) {
            qemu_cond_wait(&qemu_cpu_cond, &qemu_global_mutex); // 切换到另一个虚拟线程mttcg_cpu_thread_fn，vcpu开始执行
        }
    }
    ```
# 3. tcg_cpus_exec
    ```
    tcg_cpus_exec (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops.c:66)
    mttcg_cpu_thread_fn (/Users/kingkongtree/code/simulation/qemu-6.1.0/accel/tcg/tcg-accel-ops-mttcg.c:70)
    qemu_thread_start (/Users/kingkongtree/code/simulation/qemu-6.1.0/util/qemu-thread-posix.c:541)
    _pthread_start (@_pthread_start:40)
    ```
## 3.1 cpu_exec_start
    - 主要是为了实现smp的各种互斥加锁
## 3.2 cpu_exec // main execution loop
    - CPUState
        - nr_cores: 1
        - nr_threads: 1
        - running: true
        - stop: false
        - interrupt_request: 0
        - work_mutex: {initialized:true}
        - tb_jump_cache: {0x0000000000000000, 0x0000000000000000, ...}
        - cpu_index: 0
        - vcpu_dirty: false
    - cpu_handle_exception // 如果有exception，处理完后退出本轮执行
        - cpu_handle_interrupt // 如果有interrupt，处理完后退出本轮执行
    - tb_lookup // 根据当前PC获取tb
        - PC: 0x1000(第二个页表？)->0x80000000<_entry>->0x8000562a<start>...
        - tb_gen_code // 如果当前指令第一次执行，先翻译成host insn
        - cpu_loop_exec_tb->cpu_tb_exec->tcg_qemu_tb_exc->cpu_loop_exec_tb // 递归执行tb，

## 3.3 cpu_exec_end
    - 主要是为了实现smp的各种互斥去锁