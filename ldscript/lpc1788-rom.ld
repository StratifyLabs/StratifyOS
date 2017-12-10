

MEMORY {
   flash : ORIGIN = 0, LENGTH = 512K
   ram : ORIGIN = 0x20000000, LENGTH = 24K
   sysmem : ORIGIN = 0x20006000, LENGTH = 8K
}

SECTIONS {

   .sysmem : { 
		_sys = . ;
		*(.sysmem)
		. = ALIGN(1024);
	} >sysmem
	_esys = .;

  . = 0;
  INCLUDE ldscripts/common.ld
}

PROVIDE ( _end = . );
PROVIDE ( _iap = 0x1FFF1FF1 );
PROVIDE ( _top_of_stack = (0x20008000 - 32) );
PROVIDE ( _flash_pages = 30 );
PROVIDE ( _flash_size = 524288 );
PROVIDE ( _ram_pages = 88 );
PROVIDE ( _sram_size = 65536 );
PROVIDE ( _ahb_sram_size = 24576 );




