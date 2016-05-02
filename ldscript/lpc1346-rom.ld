
EXTERN ( core_vector_table )

MEMORY {
   flash : ORIGIN = 0, LENGTH = 48K
   ram : ORIGIN = 0x10000000, LENGTH = 6K
   sysmem : ORIGIN = 0x10001800, LENGTH = 2K
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
PROVIDE ( _top_of_stack = (0x10002000 - 32) );
PROVIDE ( _flash_pages = 10 );
PROVIDE ( _flash_size = 48*1024 );
PROVIDE ( _sram_size = 6*1024 );
PROVIDE ( _ahb_sram_size = 0*1024 );
PROVIDE ( _ctors_size = ((_dtors - _ctors)>>2) );
PROVIDE ( _dtors_size = ((_dtors_end - _dtors)>>2) );




