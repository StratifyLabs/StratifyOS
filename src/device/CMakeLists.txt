
if( ${CMSDK_BUILD_CONFIG} STREQUAL arm )
	set(SOURCES
		microchip/sst25vf_share.c
		microchip/sst25vf_ssp_tmr.c
		microchip/sst25vf_ssp.c
		microchip/sst25vf_tmr.c
		microchip/sst25vf.c
		microchip/sst25vf_local.h
		#cfifo.c
		#devfifo.c
		device_fifo.c
		ffifo.c
		fifo.c
		stream_ffifo.c
		i2s_ffifo.c
		i2s_spi_ffifo.c
		full.c
		#led_pio.c
		#led_pwm.c
		null.c
		random.c
		auth_flash.c
		drive_cfi_local.h
		drive_cfi_spi.c
		drive_cfi_qspi.c
		drive_sdspi_local.h
		drive_ram.c
		drive_mmc.c
		#drive_mmc_dma.c
		drive_sdio.c
		drive_device.c
		#drive_sdio_dma.c
		drive_sdspi.c
		#drive_sdspi_dma.c
		#drive_sdssp.c
		switchboard.c
		#tty_uart.c
		#tty_usbbulk.c
		uartfifo.c
		urandom.c
		#usbfifo.c
		zero.c
		reset_tmr.c
		PARENT_SCOPE)
endif()
