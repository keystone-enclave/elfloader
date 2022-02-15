OUTPUT_ARCH("riscv")

ENTRY( _start )

PAGE_SIZE = 0x1000; /* TODO: figure out the page size to use here */ 
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x4000; /* 4 pages */

SECTIONS
{
  
  .text :
  {
    PROVIDE( _start = . );
    *(._start)
    *(*)
  }
  
  . = ALIGN(PAGE_SIZE);
  PROVIDE ( _sstack = . );
  . = . + STACK_SIZE;
  . = ALIGN(PAGE_SIZE);
  PROVIDE ( _estack = . );
  

  /* .stack : 
  { 
    . = ALIGN(8);
    PROVIDE ( _sstack = . );
    . = . + STACK_SIZE;
    . = ALIGN(8);
    PROVIDE ( _estack = . );
  } TODO: Check that stack fits in EPM */

}
