_start:
  // a1: base addr
  // a2: base size
  // ...

  // sp <- a1 + offset
  la sp, stack_buffer
  addi sp, sp, 16*4
  addi t0, t0, 0xd

  addi sp, sp, -4
  sw   t0, 0(sp)
  lw   t1, 0(sp)

stack_buffer:
  .word 1
  .word 2
  .word 3
  .word 4
  .word 5
  .word 6
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
