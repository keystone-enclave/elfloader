toyloader:
	$(RISCV)/bin/riscv$(BITS)-unknown-elf-gcc -fPIC -nostdlib -o toy.elf toy.c
