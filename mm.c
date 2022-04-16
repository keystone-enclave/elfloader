#include "mm.h"
#include "vm_defs.h"
#include "vm.h"
#include "mem.h"

uintptr_t freeList; 
uintptr_t epmBase; 
size_t epmSize;

/* Page table utilities */
static pte*
__walk_create(pte* root, uintptr_t addr);

static uintptr_t get_new_page() {
  uintptr_t new_page = freeList; 
  if (new_page > epmBase + epmSize) {
    return -1;
  } 

  freeList += RISCV_PAGE_SIZE;
  return new_page;
}

static pte*
__continue_walk_create(pte* root, uintptr_t addr, pte* pte)
{
  uintptr_t new_page = get_new_page();

  unsigned long free_ppn = ppn(new_page);
  *pte = ptd_create(free_ppn);
  return __walk_create(root, addr);
}

static pte*
__walk_internal(pte* root, uintptr_t addr, int create)
{
  pte* t = root;
  int i;
  for (i = 1; i < RISCV_PT_LEVELS; i++)
  {
    size_t idx = RISCV_GET_PT_INDEX(addr, i);

    if (!(t[idx] & PTE_V))
      return create ? __continue_walk_create(root, addr, &t[idx]) : 0;

    t = (pte*) (pte_ppn(t[idx]) << RISCV_PAGE_BITS); 
  }

  return &t[RISCV_GET_PT_INDEX(addr, 3)];
}

/* walk the page table and return PTE
 * return 0 if no mapping exists */
static pte*
__walk(pte* root, uintptr_t addr)
{
  return __walk_internal(root, addr, 0);
}

/* walk the page table and return PTE
 * create the mapping if non exists */
static pte*
__walk_create(pte* root, uintptr_t addr)
{
  return __walk_internal(root, addr, 1);
}


uint32_t 
mapPage(uintptr_t va, uintptr_t pa) {
  pte* pte = __walk_create(root_page_table, va);

  // TODO: what is supposed to happen if page is already allocated?
  if (*pte & PTE_V) {
    return -1;
  }

  uintptr_t phys_page_num = ppn(pa);

  *pte = pte_create(phys_page_num, PTE_D | PTE_A | PTE_R | PTE_W | PTE_X | PTE_V);
  return 1;
}

uint32_t
allocPage(uintptr_t va, uintptr_t src) {
  uintptr_t page_addr;
  // uintptr_t* pFreeList = (uintptr_t*)freeList; 

  pte* pte = __walk_create(root_page_table, va);

  /* if the page has been already allocated, return the page */
  if (*pte & PTE_V) {
    return 1;
  }

  /* otherwise, allocate one from EPM freeList */
  page_addr = get_new_page();
  uintptr_t phys_page_num = ppn(page_addr);

  *pte = pte_create(phys_page_num, PTE_D | PTE_A | PTE_R | PTE_W | PTE_X | PTE_V);
  if (src != (uintptr_t) NULL) {
    memcpy((void*) page_addr, (void*) src, RISCV_PAGE_SIZE);
  } else {
    memset((void*) page_addr, 0, RISCV_PAGE_SIZE);
  }
  return 1;
}

/* This function pre-allocates the required page tables so that
 * the virtual addresses are linearly mapped to the physical memory */
size_t
epmAllocVspace(uintptr_t addr, size_t num_pages) {
  size_t count;

  for (count = 0; count < num_pages; count++, addr += RISCV_PAGE_SIZE) {
    pte* pte = __walk_create(root_page_table, addr);
    if (!pte) break;
  }

  return count;
}

uintptr_t
epm_va_to_pa(uintptr_t addr) {
  pte* pte = __walk(root_page_table, addr);
  if (pte)
    return pte_ppn(*pte) << RISCV_PAGE_BITS;
  else
    return 0;
}