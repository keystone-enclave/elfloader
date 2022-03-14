#pragma once
#include <stddef.h>
#include <stdint.h>

extern uintptr_t freeList; 
extern uintptr_t epmBase; 
extern size_t epmSize;

uint32_t mapPage(uintptr_t va, uintptr_t pa);
uint32_t allocPage(uintptr_t va, uintptr_t src);
size_t epmAllocVspace(uintptr_t addr, size_t num_pages); // TODO: unused
uintptr_t epm_va_to_pa(uintptr_t addr); // TODO: unused