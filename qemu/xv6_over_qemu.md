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
        - cpu_loop_exec_tb->cpu_tb_exec->tcg_qemu_tb_exc,最终按qemu ir来执行

## 3.3 cpu_exec_end
    - 主要是为了实现smp的各种互斥去锁
----
# 4. debug usertrap
## 4.1 Illegal Instruction
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
## 4.2 Store page fault
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