#include "elf.h"

int hello(void* i) {
    printf("runtime addr: %x\n", i);
    int sections = elf_getNumSections(i);
    printf("ELF # of sections: %d\n", sections);
    for (int cnt = 0; cnt < sections; cnt++) {
        printf("ELF section information: %s\n", elf_getSectionName(i, cnt));
    }
    printf("Eyrie runtime entry point: %x\n", elf_getEntryPoint(i));
    return 10;
}
