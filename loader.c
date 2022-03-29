#include "loader.h"
#include "csr.h"
#include "vm.h"
#include "mm.h"
#include "mem.h"
#include "printf.h"

/* For Debugging Use Only */
// static int print_pgtable(int level, pte* tb, uintptr_t vaddr)
// {
//   pte* walk;
//   int ret = 0;
//   int i=0;

//    for (walk=tb, i=0; walk < tb + ((1<<12)/sizeof(pte)) ; walk += 1, i++)
//   {
//     if(*walk == 0)
//       continue;

//      pte e = *walk;
//     uintptr_t phys_addr = (e >> 10) << 12;

//     if(level == 1 || (e & PTE_R) || (e & PTE_W) || (e & PTE_X))
//     {
//       printf("[pgtable] level:%d, base: 0x%ln, i:%d (0x%lx -> 0x%lx)\r\n", level, tb, i, ((vaddr << 9) | (i&0x1ff))<<12, phys_addr);
//     }
//     else
//     {
//       printf("[pgtable] level:%d, base: 0x%ln, i:%d, pte: 0x%lx \r\n", level, tb, i, e);
//     }

//     if(level > 1 && !(e & PTE_R) && !(e & PTE_W) && !(e & PTE_X))
//     {
//       if(level == 3 && (i&0x100))
//         vaddr = 0xffffffffffffffffUL;
//       ret |= print_pgtable(level - 1, (pte*) __va(phys_addr), (vaddr << 9) | (i&0x1ff));
//     }
//   }
//   return ret;
// }

/* Anay's code */ 
// int mapVAtoPA(uintptr_t vaddr, uintptr_t paddr, size_t size) {

//     pte app = pte_create(ppn(paddr), PTE_R | PTE_W | PTE_X);
//     load_l3_page_table[0] = app;
//     load_l2_page_table[0] = ptd_create((uintptr_t) load_l3_page_table);
//     root_page_table[0] = ptd_create((uintptr_t) load_l2_page_table);
//     // create page table by following eyrie rt
//     // alloc page and map into page table according to size
// //    uintptr_t pages = alloc_pages(vpn(vaddr), PAGE_UP(size/PAGE_SIZE), PTE_R | PTE_W | PTE_X);
// //    pte appmem = pte_create(vpn(vaddr), PTE_R | PTE_W);
//     return 0;
// }

// void csr_write_regs(uintptr_t entry_point) {
//     csr_write(satp, satp_new(__pa(root_page_table[0])));
//     csr_write(stvec, entry_point);
// }

// int hello(void* i, uintptr_t dram_base) {
//     uintptr_t minRuntimePaddr;
//     uintptr_t maxRuntimePaddr;
//     uintptr_t minRuntimeVaddr;
//     uintptr_t maxRuntimeVaddr;
//     load_pa_start = dram_base;
//     elf_getMemoryBounds(i, 1, &minRuntimePaddr, &maxRuntimePaddr);
//     elf_getMemoryBounds(i, 0, &minRuntimeVaddr, &maxRuntimeVaddr);
//     if (!IS_ALIGNED(minRuntimePaddr, PAGE_SIZE)) {
//         return false;
//     }
// /*    if (loadElf(i)) {
//         return false;
//     }*/
//     int status = mapVAtoPA(minRuntimeVaddr, minRuntimePaddr, 0 /* size */);
//     if (status != 0) {
//        return 1;
//     }
//     print_pgtable(0, root_page_table, minRuntimeVaddr);
//     return 10;
// }

int test(int i) {
  return i + 1; 
}

void initializeFreeList(uintptr_t freeMemBase, uintptr_t dramBase, size_t dramSize) {
  printf("Initializing free list\n");
  freeList = freeMemBase;
  epmBase = dramBase; 
  epmSize = dramSize;
  printf("Finished initializing free list\n");
}

int loadElf(elf_t* elf) {
  printf("Loading elf\n");

  for (unsigned int i = 0; i < elf_getNumProgramHeaders(elf); i++) {
    if (elf_getProgramHeaderType(elf, i) != PT_LOAD) {
      continue;
    }

    uintptr_t start      = elf_getProgramHeaderVaddr(elf, i);
    uintptr_t file_end   = start + elf_getProgramHeaderFileSize(elf, i);
    uintptr_t memory_end = start + elf_getProgramHeaderMemorySize(elf, i);
    char* src            = (char*)(elf_getProgramSegment(elf, i));
    uintptr_t va         = start;

    printf("Loading initialized segment for program header %d\n", i);
    /* first load all pages that do not include .bss segment */
    while (va + RISCV_PAGE_SIZE <= file_end) {
      if (!mapPage(va, (uintptr_t)src))
        //return Error::PageAllocationFailure;
        return -1; //TODO: error class later
      src += RISCV_PAGE_SIZE;
      va += RISCV_PAGE_SIZE;
    }

    /* next, load the page that has both initialized and uninitialized segments
     */
    if (va < file_end) {
      if (!mapPage(va, (uintptr_t) src))
        //return Error::PageAllocationFailure;
        return -1; //TODO: error class later
      va += RISCV_PAGE_SIZE;
    }

    printf("Loading .bss segment for program header %d\n", i);
    /* finally, load the remaining .bss segments */
    while (va < memory_end) {
      if (!allocPage(va, (uintptr_t) NULL))
        //return Error::PageAllocationFailure;
        return -1; //TODO: error class later
      va += RISCV_PAGE_SIZE;
    }
  }

   //return Error::Success;
   return 0; //TODO: error class later
}

/* Loader is for Sv39 */
uintptr_t satp_new(uintptr_t pa)
{
  printf("Create new satp\n");
  return (SATP_MODE | (pa >> RISCV_PAGE_BITS));
}

int load_runtime(uintptr_t dram_base, uintptr_t dram_size, 
                      uintptr_t runtime_base, uintptr_t user_base, 
                      uintptr_t free_base, uintptr_t untrusted_ptr, 
                      uintptr_t untrusted_size) {
  int ret = 0;

  // initialize free list
  initializeFreeList(free_base, dram_base, dram_size);

  // validate runtime elf 
  size_t runtime_size = user_base - runtime_base;
  if (((void*) runtime_base == NULL) || (runtime_size <= 0)) {
    return -1; 
  }

  // create runtime elf struct
  elf_t runtime_elf;
  ret = elf_newFile((void*) runtime_base, runtime_size, &runtime_elf);
  if (ret < 0) {
    return ret;
  }

  // map runtime memory
  ret = loadElf(&runtime_elf);
  return ret;
}