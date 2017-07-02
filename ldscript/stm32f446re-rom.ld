
EXTERN ( core_vector_table )

MEMORY {
   flash : ORIGIN = 0x08000000, LENGTH = 512K
   ram : ORIGIN = 0x20000000, LENGTH = 112K
   sysmem : ORIGIN = 0x2001C000, LENGTH = 16K
}

__ram_size__ = 24K;

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
PROVIDE ( _top_of_stack = (0x2001C000 + 16384) );
PROVIDE ( _estack = (0x2001C000 + 16384) );
PROVIDE ( _flash_pages = 30 );
PROVIDE ( _flash_size = 524288 );
PROVIDE ( _ram_pages = 88 );
PROVIDE ( _sram_size = 65536 );
PROVIDE ( _ahb_sram_size = 0 );




