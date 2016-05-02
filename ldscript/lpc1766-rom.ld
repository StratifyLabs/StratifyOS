
EXTERN ( core_vector_table )

MEMORY {
   flash : ORIGIN = 0, LENGTH = 256K
   ram : ORIGIN = 0x2007C000, LENGTH = 28K
   sysmem : ORIGIN = 0x20083000, LENGTH = 4K
}

__ram_size__ = 28K;

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
PROVIDE ( _top_of_stack = (0x20084000 - 32) );
PROVIDE ( _flash_pages = 22 );
PROVIDE ( _flash_size = 256*1024 );
PROVIDE ( _sram_size = 32768 );
PROVIDE ( _ahb_sram_size = 28672 );




