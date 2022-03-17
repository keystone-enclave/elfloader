#include "csr.h"

#define SBI_EXT_EXPERIMENTAL_KEYSTONE_ENCLAVE 0x08424b45
#define SBI_SM_EXIT_ENCLAVE      3006
#define RUNTIME_ADDRESS 0xffffffffc0000000 

#if __riscv_xlen == 64
#define STORE sd
#define LOAD ld
#define LOG_REGBYTES  3
#define WORD .dword
#elif __riscv_xlen == 32
#define STORE sw
#define LOAD lw
#define LOG_REGBYTES  2
#define WORD .word
#endif

.section .text
_start:
  // a1: dram_base
  // a2: dram_size
  // a3: runtime_base
  // a4: user_base
  // a5: free_base
  // a6: untrusted_ptr
  // a7: untrusted_size
  // ...

  // sp <- stack_addr + dram_base (a1)
  la sp, _estack
  # add sp, sp, a1

  // save all args to stack 
  addi sp, sp, -28
  sw a1, 0(sp)
  sw a2, 4(sp)
  sw a3, 8(sp)
  sw a4, 12(sp)
  sw a5, 16(sp)
  sw a6, 20(sp)
  sw a7, 24(sp)

  # // test function
  # li a0, 5
  # call test

  // set the free memory
  mv a0, a5 // a1, a2 already dram_base and dram_size
  la a3, freeList 
  add a3, a3, a1 
  la a4, epmBase 
  add a4, a4, a1 
  la a5, epmSize 
  add a5, a5, a1 
  call initializeFreeList  

  // validate runtime  
  lw a3, 8(sp) // set a3 to runtime_base
  beq a3, x0, exit // check that pointer is not null
  lw a4, 12(sp) // set a4 to user_base
  sub t0, a4, a3 
  blt t0, x0, exit // check file size is greater than zero 

  // call elf_newFile 
  mv a0, a3 
  mv a1, t0 
  addi sp, sp, -12
  mv a2, sp
  call elf_newFile
  
  // call load elf to map runtime
  mv a0, sp
  call loadElf 

  // exit if errors
  blt a0, x0, exit

  // switch to va and jump to runtime code 
  li t0, RUNTIME_ADDRESS 
  csrw stvec, t0  // store runtime start addresss in stvec 

  la a0, root_page_table  
  call satp_new // construct new satp

  csrw satp, a0 // switch to virtual addresssing 


exit: // exit enclave
  li a7, SBI_EXT_EXPERIMENTAL_KEYSTONE_ENCLAVE
  li a6, SBI_SM_EXIT_ENCLAVE
  ecall
  
_sstack:
  .dword 1
  .dword 2
  .dword 3
  .dword 4
  .dword 5
  .dword 6
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
  .dword 0
_estack: