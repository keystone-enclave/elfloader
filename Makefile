CROSS_COMPILE = riscv64-unknown-linux-gnu-# riscv$(BITS)-unknown-linux-gnu-
CC = $(CROSS_COMPILE)gcc
ELF_CROSS_COMPILE = riscv64-unknown-elf-
ELF_CC = $(ELF_CROSS_COMPILE)gcc
ELF_OBJCOPY = $(ELF_CROSS_COMPILE)objcopy



# ifndef KEYSTONE_SDK_DIR
#   $(error KEYSTONE_SDK_DIR is undefined)
# endif

CFLAGS = -Wall -Werror -fPIC -fno-builtin -std=c11 -g $(OPTIONS_FLAGS)
SRCS = elf.c elf32.c elf64.c loader.c printf.c sbi.c sbi.c 
ASM_SRCS = loader.S
LOADER = loader
# LINK = $(CROSS_COMPILE)ld
LDFLAGS = -static-pie -nostdlib $(shell $(CC) --print-file-name=libgcc.a)

# SDK_LIB_DIR = $(KEYSTONE_SDK_DIR)/lib
# SDK_INCLUDE_EDGE_DIR = $(KEYSTONE_SDK_DIR)/include/edge # Why does it use the edge library? (edge calls?) 
# SDK_EDGE_LIB = $(SDK_LIB_DIR)/libkeystone-edge.a

# LDFLAGS += -L$(SDK_LIB_DIR)
# CFLAGS += -I$(SDK_INCLUDE_EDGE_DIR)

OBJS = $(patsubst %.c,obj/%.o,$(SRCS))
ASM_OBJS = $(patsubst %.S,obj/%.S.o,$(ASM_SRCS))
OBJ_DIR_EXISTS = obj/.exists
 
.PHONY: all clean

all: $(LOADER)

$(LOADER): $(ASM_OBJS) $(OBJS) 
	$(ELF_CC) -nostdlib -t loader.lds $^ -o $@
	$(ELF_OBJCOPY) -O binary --only-section .text $@ loader.bin

$(OBJ_DIR_EXISTS):
	mkdir -p obj
	touch $(OBJ_DIR_EXISTS)

obj/%.S.o: %.S $(OBJ_DIR_EXISTS)
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: %.c $(OBJ_DIR_EXISTS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj 
	rm $(LOADER) $(LOADER).bin
