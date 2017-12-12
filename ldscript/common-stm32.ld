

  .text : {
         _text = .;
  		 KEEP(*(.startup))
  		 KEEP(*(.reset_vector))
  		 KEEP(*(.isr_vector))
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
         _priv_code = .;
         *(.priv_code)
         _priv_exec_code = .;
         *(.priv_exec_code)
         _epriv_code = .;
  } >flash
         
         
  .ARM.extab : 
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > flash
  __exidx_start = .;
  .ARM.exidx   : { 
    *(.ARM.exidx* .gnu.linkonce.armexidx.*) 
  } >flash
  
  . = ALIGN(4);
  __exidx_end = .;
  _etext = .;
  .data : {
         _data = .;
         *(.data)
         *(.data.*)
         } >ram AT>flash
  _edata = .;
  .bss : {
    	_bss = .;
         *(.bss)
         *(.bss.*)
         } >ram
  . = ALIGN(4);
  _ebss = .;





