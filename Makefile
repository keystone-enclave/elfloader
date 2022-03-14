CROSS_COMPILE = riscv64-unknown-linux-gnu-# riscv$(BITS)-unknown-linux-gnu-
CC = $(CROSS_COMPILE)gcc
ELF_CROSS_COMPILE = riscv64-unknown-elf-
ELF_CC = $(ELF_CROSS_COMPILE)gcc
ELF_OBJCOPY = $(ELF_CROSS_COMPILE)objcopy



# ifndef KEYSTONE_SDK_DIR
#   $(error KEYSTONE_SDK_DIR is undefined)
# endif

CFLAGS = -Wall -Werror -fPIC -fno-builtin -nostdlib -g $(OPTIONS_FLAGS)
SRCS = mem.c string.c printf.c elf.c elf32.c elf64.c loader.c mm.c sbi.c sbi.c vm.c
ASM_SRCS = loader.S
LOADER = loader
LOADER_BIN = loader.bin
# LINK = $(CROSS_COMPILE)ld
LDFLAGS = -fPIC -nostdlib $(shell $(CC) --print-file-name=libgcc.a)

# SDK_LIB_DIR = $(KEYSTONE_SDK_DIR)/lib
# SDK_INCLUDE_EDGE_DIR = $(KEYSTONE_SDK_DIR)/include/edge # Why does it use the edge library? (edge calls?) 
# SDK_EDGE_LIB = $(SDK_LIB_DIR)/libkeystone-edge.a

# LDFLAGS += -L$(SDK_LIB_DIR)
# CFLAGS += -I$(SDK_INCLUDE_EDGE_DIR)

OBJS = $(patsubst %.c,obj/%.o,$(SRCS))
ASM_OBJS = $(patsubst %.S,obj/%.S.o,$(ASM_SRCS))
OBJ_DIR_EXISTS = obj/.exists
 
.PHONY: all clean

all: $(LOADER_BIN)

$(LOADER): $(ASM_OBJS) $(OBJS) 
	$(ELF_CC) $(LDFLAGS) -t loader.lds $^ -o $@

$(LOADER_BIN): $(LOADER) 
	$(ELF_OBJCOPY) -O binary --only-section .text $< loader.bin

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
