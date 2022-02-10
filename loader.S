.section .text
_start:
  // a1: base addr
  // a2: base size
  // ...

  // sp <- a1 + offset
  la sp, _estack
  addi t0, t0, 0xd

  addi sp, sp, -4
  sw   t0, 0(sp)
  lw   t1, 0(sp)
  mv a0, a3
  call hello
  mv a0, a4
  call hello
  // exit enclave
  li a7, 0x08424b45
  li a6, 3006
  ecall
  
