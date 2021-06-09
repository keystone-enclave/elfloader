toyloader:
	riscv64-unknown-elf-gcc -nostdlib -fPIC -t toy.lds toy.S toy.c -o toy.elf
	riscv64-unknown-elf-objcopy -O binary --only-section .text toy.elf toy.bin


