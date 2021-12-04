OBJECT_FILES := loader.S.o loader.c.o sbi.c.o printf.c.o elf64.c.o elf32.c.o elf.c.o
CC := riscv64-unknown-linux-gnu-gcc

%.S.o: %.S
	$(CC) -c $< -o $@

%.c.o: %.c
	$(CC) -g -fPIC -c $< -o $@

loader: $(OBJECT_FILES)
	riscv64-unknown-elf-gcc -nostdlib -t loader.lds $(OBJECT_FILES) -o loader.elf
	riscv64-unknown-elf-objcopy -O binary --only-section .text loader.elf loader.bin

clean:
	rm *.o loader.elf loader.bin
