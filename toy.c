char stackarr[100] = {'0'};

void
_start() {
        asm("la sp, stackarr");
	unsigned int a = 7;
	unsigned int b = 3;
	unsigned int result = a + b;
//	return result;
}
