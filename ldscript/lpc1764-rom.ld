
EXTERN ( core_vector_table )

MEMORY {
   flash : ORIGIN = 0, LENGTH = 128K
   ram : ORIGIN = 0x2007C000, LENGTH = 12K
   sysmem : ORIGIN = 0x2007F000, LENGTH = 4K
}

__ram_size__ = 12K;

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
PROVIDE ( _top_of_stack = (0x20080000 - 32) );
PROVIDE ( _flash_pages = 18 );
PROVIDE ( _flash_size = 128*1024 );
PROVIDE ( _sram_size = 16*1024 );
PROVIDE ( _ahb_sram_size = 12*1024 );




