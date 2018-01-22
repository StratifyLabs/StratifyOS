

if( ${SOS_BUILD_CONFIG} STREQUAL arm )
    set(SOURCES ${SOURCES}
      ${SOURCES_PREFIX}/adc.c
      ${SOURCES_PREFIX}/core.c
      ${SOURCES_PREFIX}/dac_dma.c
      ${SOURCES_PREFIX}/dac.c
      ${SOURCES_PREFIX}/eeprom.c
      ${SOURCES_PREFIX}/eint.c
      ${SOURCES_PREFIX}/emc.c
      ${SOURCES_PREFIX}/enet.c
      ${SOURCES_PREFIX}/flash.c
      ${SOURCES_PREFIX}/i2c.c
      ${SOURCES_PREFIX}/i2s.c
      ${SOURCES_PREFIX}/link_transport_slave.c
      ${SOURCES_PREFIX}/mci.c
      ${SOURCES_PREFIX}/mcu_debug.c
      ${SOURCES_PREFIX}/mcu_sync_io.c
      ${SOURCES_PREFIX}/mcu.c
      ${SOURCES_PREFIX}/mem.c
      ${SOURCES_PREFIX}/pio.c
      ${SOURCES_PREFIX}/pwm.c
      ${SOURCES_PREFIX}/qei.c
      ${SOURCES_PREFIX}/rtc.c
      ${SOURCES_PREFIX}/spi.c
      ${SOURCES_PREFIX}/ssp.c
      ${SOURCES_PREFIX}/sys.c
      ${SOURCES_PREFIX}/tmr.c
      ${SOURCES_PREFIX}/uart.c
      ${SOURCES_PREFIX}/usb.c
      PARENT_SCOPE)
  endif()
