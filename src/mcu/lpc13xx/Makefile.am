AM_CFLAGS=-I$(srcdir)/../include -Os -DF_CPU=12000000UL -D__CoActionOS__ -D__lpc13xx -ffunction-sections

lib_LTLIBRARIES = libhwpl_lpc13xx.la
libhwpl_lpc13xx_la_CFLAGS = $(CFLAGS) $(AM_CFLAGS)
libhwpl_lpc13xx_ladir = $(includedir)/hwpl/arch
libhwpl_lpc13xx_la_HEADERS = ../include/hwpl/arch/lpc13xx.h \
	../include/hwpl/arch/lpc13xx_flags.h	

libhwpl_lpc13xx_la_SOURCES = ../core_cm3/core_cm3.c \
	../core_cm3/fault.c \
	../core_cm3/task_flags.h \
	../core_cm3/task.c \
	../core_cm3/task_process.c \
	../core_cm3/task_single.c \
	../core_cm3/task_mpu.c \
	../core_cm3/mpu.c \
	../core_cm3/priv.c \
	./core/core_set_clock.c	\
	./core/core_set_usb_clock.c \
	./core/core_flags.h \
	./core/core_wr_pinmode.c \
	./core/core_sleep.c \
	./core/fault_dev.c \
	./core/mpu_dev.c \
	./core_startup.c \
	./core_set_pinsel_func.c \
	./core_wdt.c \
	./core_wdt_flags.h \
	./adc_dev.c \
	./core_dev.c \
	./flash_dev.c \
	./pio_dev.c \
	./hwpl_dev.c \
	./i2c_dev.c \
	./spi_dev.c \
	./tmr_dev.c \
	./uart_dev.c \
	./usb_dev.c \
	../share/lpc_flash.c \
	../share/lpc_flash_flags.h \
	../share/adc.c \
	../share/core.c \
	../share/flash.c \
	../share/pio.c \
	../share/i2c.c \
	../share/spi.c \
	../share/tmr.c \
	../share/uart.c \
	../share/usb.c \
	../share/circ_buf.c \
	../share/circ_buf_find_byte.c