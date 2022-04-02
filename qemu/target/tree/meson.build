# FIXME extra_args should accept files()
dir = meson.current_source_dir()

gen = [
  decodetree.process('insn16.decode', extra_args: ['--static-decode=decode_insn16', '--insnwidth=16']),
  decodetree.process('insn32.decode', extra_args: '--static-decode=decode_insn32'),
]

tree_ss = ss.source_set()
tree_ss.add(gen)
tree_ss.add(files(
  'cpu.c',
  'cpu_helper.c',
  'csr.c',
  'fpu_helper.c',
  'gdbstub.c',
  'op_helper.c',
  'vector_helper.c',
  'bitmanip_helper.c',
  'translate.c',
))

tree_softmmu_ss = ss.source_set()
tree_softmmu_ss.add(files(
  'arch_dump.c',
  'pmp.c',
  'monitor.c',
  'machine.c'
))

target_arch += {'tree': tree_ss}
target_softmmu_arch += {'tree': tree_softmmu_ss}
