

MEMORY {
   flash : ORIGIN = 0xDE000000, LENGTH = 8M
   ram : ORIGIN = 0xDE800000, LENGTH = 8355840
   kernel : ORIGIN = 0xDEFF8000, LENGTH = 32K
}

__ram_size__ = 32K;



SECTIONS {

  . = 0;
  .text : {
  		 _text = .;
  		 *(.startup)
         *(.text)
         *(.init)
         *(.fini)
         *(.eh_frame)
         _ctors = .;
         KEEP(*(.init_array))
         KEEP(*(.ctors))
         _dtors = .;
         KEEP(*(.dtors))
         KEEP(*(.fini_array))
         _dtors_end = .;         
         *(.jcr)
         *(.rodata)
         *(.rodata*)
         *(.glue_7)
         *(.glue_7t)
         *(.text.*)
         KEEP(*(_binary___*))

				 PROVIDE(__start_sdh_soc_observers = .);
				 KEEP(*(SORT(.sdh_soc_observers*)))
				 PROVIDE(__stop_sdh_soc_observers = .);
				 PROVIDE(__start_sdh_ble_observers = .);
				 KEEP(*(SORT(.sdh_ble_observers*)))
				 PROVIDE(__stop_sdh_ble_observers = .);
				 PROVIDE(__start_sdh_stack_observers = .);
				 KEEP(*(SORT(.sdh_stack_observers*)))
				 PROVIDE(__stop_sdh_stack_observers = .);
				 PROVIDE(__start_sdh_req_observers = .);
				 KEEP(*(SORT(.sdh_req_observers*)))
				 PROVIDE(__stop_sdh_req_observers = .);
				 PROVIDE(__start_sdh_state_observers = .);
				 KEEP(*(SORT(.sdh_state_observers*)))
				 PROVIDE(__stop_sdh_state_observers = .);
				 PROVIDE(__start_nrf_queue = .);
				 KEEP(*(.nrf_queue))
				 PROVIDE(__stop_nrf_queue = .);
         
         FILL(0xFF);
         
	 } > flash

   
  .ARM.extab : 
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > flash
	
  __exidx_start = .;
  .ARM.exidx   : { 
    *(.ARM.exidx* .gnu.linkonce.armexidx.*) 
  } >flash
  __exidx_end = .;
         
  _etext = .;
  
  .data : {
         _data = .;
         __data_beg_src__ = _etext;
         *(.global_reent)
         *(.data)
         *(.data.*)
            . = ALIGN(4);
         } >ram AT>flash
  _edata = .;
  _bss = .;
  .bss : {
         *(.bss)
         *(.bss.*)
         } >ram
   . = ALIGN(4);
   _ebss = .;
}

PROVIDE ( _text_size = (_etext - _text) );
PROVIDE ( _data_size = (_edata - _data) );
PROVIDE ( _ctors_size = ((_dtors - _ctors)>>2) );
PROVIDE ( _dtors_size = ((_dtors_end - _dtors)>>2) );






