OUTPUT_ARCH("riscv")

ENTRY( _start )

STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x800;

SECTIONS
{
  . = 0x0;

  .text :
  {
    PROVIDE( _start = . );
    *(._start)
    *(*)
  }

  .stack : 
  { 
    . = ALIGN(8);
    _sstack = .;
    . = . + STACK_SIZE;
    . = ALIGN(8);
    _estack = .;
  } /* TODO: Check that stack fits in EPM */

}
