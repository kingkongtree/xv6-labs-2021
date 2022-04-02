tree_ss = ss.source_set()

tree_ss.add(files('boot.c'), fdt)
tree_ss.add(files('riscv_hart.c'))
tree_ss.add(when: 'CONFIG_RISCV_NUMA', if_true: files('numa.c'))
tree_ss.add(when: 'CONFIG_RISCV_VIRT', if_true: files('virt.c'))

hw_arch += {'tree': tree_ss}
