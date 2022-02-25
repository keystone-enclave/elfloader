static int allocContinguousVspace(uintptr_t va, size_t num_pages) {
  return 0;
}

static int allocPage(uintptr_t va, uintptr_t src_pa) {
  return 0;
}

static bool mapElf(ElfFile* elf) {
  uintptr_t va;

  assert(elf);

  size_t num_pages =
      ROUND_DOWN(elf->getTotalMemorySize(), PAGE_BITS) / PAGE_SIZE;
  va = elf->getMinVaddr();

  if (pMemory->epmAllocVspace(va, num_pages) != num_pages) {
    ERROR("failed to allocate vspace\n");
    return false;
  }

  return true;
}

static int loadElf(ElfFile* elf) {
  static char nullpage[PAGE_SIZE] = {
      0,
  };

  unsigned int mode = elf->getPageMode();
  for (unsigned int i = 0; i < elf->getNumProgramHeaders(); i++) {
    if (elf->getProgramHeaderType(i) != PT_LOAD) {
      continue;
    }

    uintptr_t start      = elf->getProgramHeaderVaddr(i);
    uintptr_t file_end   = start + elf->getProgramHeaderFileSize(i);
    uintptr_t memory_end = start + elf->getProgramHeaderMemorySize(i);
    char* src            = reinterpret_cast<char*>(elf->getProgramSegment(i));
    uintptr_t va         = start;

    /* FIXME: This is a temporary fix for loading iozone binary
     * which has a page-misaligned program header. */
    if (!IS_ALIGNED(va, PAGE_SIZE)) {
      size_t offset = va - PAGE_DOWN(va);
      size_t length = PAGE_UP(va) - va;
      char page[PAGE_SIZE];
      memset(page, 0, PAGE_SIZE);
      memcpy(page + offset, (const void*)src, length);
      if (!pMemory->allocPage(PAGE_DOWN(va), (uintptr_t)page, mode))
        return Error::PageAllocationFailure;
      va += length;
      src += length;
    }

    /* first load all pages that do not include .bss segment */
    while (va + PAGE_SIZE <= file_end) {
      if (!pMemory->allocPage(va, (uintptr_t)src, mode))
        return Error::PageAllocationFailure;

      src += PAGE_SIZE;
      va += PAGE_SIZE;
    }

    /* next, load the page that has both initialized and uninitialized segments
     */
    if (va < file_end) {
      char page[PAGE_SIZE];
      memset(page, 0, PAGE_SIZE);
      memcpy(page, (const void*)src, static_cast<size_t>(file_end - va));
      if (!pMemory->allocPage(va, (uintptr_t)page, mode))
        return Error::PageAllocationFailure;
      va += PAGE_SIZE;
    }

    /* finally, load the remaining .bss segments */
    while (va < memory_end) {
      if (!pMemory->allocPage(va, (uintptr_t)nullpage, mode))
        return Error::PageAllocationFailure;
      va += PAGE_SIZE;
    }
  }

  return Error::Success;
}