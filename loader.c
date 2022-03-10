#include "loader.h"
#include "csr.h"
#include "vm.h"
#include "printf.h"
#include "elf32.h"

pte root_page_table[BIT(RISCV_PT_INDEX_BITS)] __attribute__((aligned(RISCV_PAGE_SIZE)));
pte load_l2_page_table[BIT(RISCV_PT_INDEX_BITS)] __attribute__((aligned(RISCV_PAGE_SIZE)));
pte load_l3_page_table[BIT(RISCV_PT_INDEX_BITS)] __attribute__((aligned(RISCV_PAGE_SIZE)));
uintptr_t load_pa_start;

static int print_pgtable(int level, pte* tb, uintptr_t vaddr)
{
  pte* walk;
  int ret = 0;
  int i=0;

   for (walk=tb, i=0; walk < tb + ((1<<12)/sizeof(pte)) ; walk += 1, i++)
  {
    if(*walk == 0)
      continue;

     pte e = *walk;
    uintptr_t phys_addr = (e >> 10) << 12;

    if(level == 1 || (e & PTE_R) || (e & PTE_W) || (e & PTE_X))
    {
      printf("[pgtable] level:%d, base: 0x%ln, i:%d (0x%lx -> 0x%lx)\r\n", level, tb, i, ((vaddr << 9) | (i&0x1ff))<<12, phys_addr);
    }
    else
    {
      printf("[pgtable] level:%d, base: 0x%ln, i:%d, pte: 0x%lx \r\n", level, tb, i, e);
    }

    if(level > 1 && !(e & PTE_R) && !(e & PTE_W) && !(e & PTE_X))
    {
      if(level == 3 && (i&0x100))
        vaddr = 0xffffffffffffffffUL;
      ret |= print_pgtable(level - 1, (pte*) __va(phys_addr), (vaddr << 9) | (i&0x1ff));
    }
  }
  return ret;
}

int mapVAtoPA(uintptr_t vaddr, uintptr_t paddr, size_t size) {

    pte app = pte_create(ppn(paddr), PTE_R | PTE_W | PTE_X);
    load_l3_page_table[0] = app;
    load_l2_page_table[0] = ptd_create((uintptr_t) load_l3_page_table);
    root_page_table[0] = ptd_create((uintptr_t) load_l2_page_table);
    // create page table by following eyrie rt
    // alloc page and map into page table according to size
//    uintptr_t pages = alloc_pages(vpn(vaddr), PAGE_UP(size/PAGE_SIZE), PTE_R | PTE_W | PTE_X);
//    pte appmem = pte_create(vpn(vaddr), PTE_R | PTE_W);
    return 0;
}

void csr_write_regs(uintptr_t entry_point) {
    csr_write(satp, satp_new(__pa(root_page_table[0])));
    csr_write(stvec, entry_point);
}

int hello(void* i, uintptr_t dram_base) {
    uintptr_t minRuntimePaddr;
    uintptr_t maxRuntimePaddr;
    uintptr_t minRuntimeVaddr;
    uintptr_t maxRuntimeVaddr;
    load_pa_start = dram_base;
    elf_getMemoryBounds(i, 1, &minRuntimePaddr, &maxRuntimePaddr);
    elf_getMemoryBounds(i, 0, &minRuntimeVaddr, &maxRuntimeVaddr);
    if (!IS_ALIGNED(minRuntimePaddr, PAGE_SIZE)) {
        return false;
    }
/*    if (loadElf(i)) {
        return false;
    }*/
    int status = mapVAtoPA(minRuntimeVaddr, minRuntimePaddr, 0 /* size */);
    if (status != 0) {
       return 1;
    }
    print_pgtable(0, root_page_table, minRuntimeVaddr);
    return 10;
}

int test(int i) {
  return i + 1; 
}

// returns list of dynamic libraries into dyn_list 
// return value holds the number of dynamic libraries or value < 0 if error
int find_dynamic_libraries(uintptr_t eapp_elf, size_t eapp_elf_size, char **dyn_list) {
  int ret = 0; 

  elf_t elf_object;
  ret = elf_newFile((void*) eapp_elf, eapp_elf_size, &elf_object);

  // Parse the ELF header, check that the file is a dynamic executable (ET_EXEC or ET_DYN).
  int16_t e_type = elf_getEtype(&elf_object);
  if (!(e_type == ET_EXEC || e_type == ET_DYN)) {
    ret = -1; // TODO: add more specific error 
  }

  // parse the program headers, looking for the PT_DYNAMIC one. Also remember virtual address -> file offset mappings for the PT_LOAD segments.
  int num_phdrs = elf_getNumProgramHeaders(&elf_object);
  for (uint32_t i = 0; i < num_phdrs; i++) {
    
    if (elf_getProgramHeaderType(&elf_object, i) == PT_DYNAMIC) {
      // Once found, parse the dynamic section. Look for the DT_NEEDED and DT_STRTAB entries.
      uintptr_t dynamic_offset = elf_getProgramHeaderOffset(&elf_object, i);

      if (elf_isElf32(&elf_object)) {

        size_t dyn_elem_size = sizeof(Elf32_Dyn);
        uintptr_t dyn_entry = (uintptr_t) elf_object.elfFile + dynamic_offset; 
      
        uintptr_t strtab_addr;
        uint32_t dlib_offsets[32]; // TODO: fix this use of magic number
        uint32_t num_dlibs = 0;

        do {
          if (((Elf32_Dyn*) dyn_entry)->d_tag == DT_STRTAB) {
            strtab_addr = ((Elf32_Dyn*) dyn_entry)->d_un.d_ptr;
          }
          if (((Elf32_Dyn*) dyn_entry)->d_tag == DT_NEEDED) {
            dlib_offsets[i] = ((Elf32_Dyn*) dyn_entry)->d_un.d_val;
            num_dlibs++;
          }
          dyn_entry += dyn_elem_size;
        } while (((Elf32_Dyn*) dyn_entry)->d_tag != DT_NULL);

        // Look for library names in STRTAB, and add them to dyn_list
        for (int j = 0; j < num_dlibs; j++) {
          *(dyn_list + j*sizeof(char*)) = (char *) strtab_addr + dlib_offsets[j];
        }
        ret = num_dlibs;
        /* DEBUG ONLY */ 
        for (int k = 0; k < num_dlibs; k++) {
          printf("%dth dynamic library: %s", k, (char*) (dyn_list + k*sizeof(char*)));
        }
      } else {
        size_t dyn_elem_size = sizeof(Elf64_Dyn);
        uintptr_t dyn_entry = (uintptr_t) elf_object.elfFile + dynamic_offset; 
      
        uintptr_t strtab_addr;
        uint32_t dlib_offsets[32]; // TODO: fix this use of magic number
        uint32_t num_dlibs = 0;

        do {
          if (((Elf64_Dyn*) dyn_entry)->d_tag == DT_STRTAB) {
            strtab_addr = ((Elf64_Dyn*) dyn_entry)->d_un.d_ptr;
          }
          if (((Elf64_Dyn*) dyn_entry)->d_tag == DT_NEEDED) {
            dlib_offsets[i] = ((Elf64_Dyn*) dyn_entry)->d_un.d_val;
            num_dlibs++;
          }
          dyn_entry += dyn_elem_size;
        } while (((Elf64_Dyn*) dyn_entry)->d_tag != DT_NULL);

        // Look for library names in STRTAB, and add them to dyn_list
        for (int j = 0; j < num_dlibs; j++) {
          *(dyn_list + j*sizeof(char)) = (char *) strtab_addr + dlib_offsets[j];
        }
        ret = num_dlibs;
        /* DEBUG ONLY */ 
        for (int k = 0; k < num_dlibs; k++) {
          printf("%dth dynamic library: %s", k, (char*) (dyn_list + k*sizeof(char*)));
        }
      }
    }
  }
  return ret;
}