#include "elf.h"
#include "string.h"

// method definitions
// extern int hello(void * i, uintptr_t dram_base);
extern void initializeFreeList(uintptr_t freeMemBase, uintptr_t dramBase, size_t dramSize);
extern int loadElf(elf_t* elf);
