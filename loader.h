#include "elf.h"
#include "string.h"

// method definitions
// extern int hello(void * i, uintptr_t dram_base);
// extern void initializeFreeList(uintptr_t freeMemBase, uintptr_t dramBase, size_t dramSize);
void initializeFreeList(uintptr_t freeMemBase, uintptr_t dramBase, size_t dramSize, 
    uintptr_t* freeList, uintptr_t* epmBase, uintptr_t* epmSize);
extern int loadElf(elf_t* elf);
extern uintptr_t satp_new(uintptr_t pa);
