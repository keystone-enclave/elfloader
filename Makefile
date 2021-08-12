OBJECT_FILES := toy.S.o toy.c.o sbi.c.o printf.c.o elf64.c.o elf32.c.o elf.c.o
CC := riscv64-unknown-elf-gcc

%.S.o: %.S
	$(CC) -c $< -o $@

%.c.o: %.c
	$(CC) -g -fPIC -c $< -o $@

toyloader: $(OBJECT_FILES)
	riscv64-unknown-elf-gcc -nostdlib -t toy.lds $(OBJECT_FILES) -o toy.elf
	riscv64-unknown-elf-objcopy -O binary --only-section .text toy.elf toy.bin
