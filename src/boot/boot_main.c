
#include <sys/lock.h>

#include "sos/arch.h"
#include "sos/symbols.h"

#include "boot_config.h"
#include "boot_link.h"
#include "cortexm/cortexm.h"
#include "sos/config.h"
#include "sos/debug.h"
#include "sos/dev/usb.h"
#include "sos/fs/devfs.h"
#include "sos/led.h"
#include "sos/sos.h"
#include "usbd/control.h"

int boot_main();

void boot_invoke_bootloader(void *args) {
  // write SW location with key and then reset
  u32 *bootloader_software_request_address =
    (u32 *)sos_config.boot.software_bootloader_request_address;
  *bootloader_software_request_address =
    sos_config.boot.software_bootloader_request_value;
  cortexm_reset(0);
}

void sos_handle_event(int event, void *args) {
  if (sos_config.event_handler != NULL) {
    sos_config.event_handler(event, args);
  }
}

void cortexm_reset_handler() {
  u32 *src, *dest;
  src = &_etext; // point src to copy of data that is stored in flash
  for (dest = &_data; dest < &_edata;) {
    *dest++ = *src++;
  } // Copy from flash to RAM (data)
  for (src = &_bss; src < &_ebss;)
    *src++ = 0; // Zero out BSS section
  for (src = &_sys; src < &_esys;)
    *src++ = 0; // Zero out sysmem

  boot_main(); // This function should never return
}

extern u32 _etext;

const bootloader_api_t mcu_core_bootloader_api = {
  .code_size = (u32)&_etext,
  .exec = boot_invoke_bootloader,
  .event = sos_handle_event};

void init_hw();

static void *stack_ptr;
static void (*app_reset)();

void run_bootloader();

void led_flash() {
  sos_led_svcall_enable(0);
  cortexm_delay_ms(500);
  sos_led_svcall_disable(0);
  cortexm_delay_ms(500);
}

void led_flash_run_bootloader() {
  int i;
  for (i = 0; i < 3; i++) {

    sos_led_svcall_enable(0);
    cortexm_delay_ms(50);
    sos_led_svcall_disable(0);
    cortexm_delay_ms(50);
  }
}

static int check_run_app();
void run_bootloader();

/*! \details
 */
int boot_main() {

  sos_handle_event(SOS_EVENT_ROOT_RESET, 0);

  stack_ptr = (void *)(((u32 *)sos_config.boot.program_start_address)[0]);
  app_reset = (void (*)())((((u32 *)sos_config.boot.program_start_address)[1]));

  if (check_run_app()) {
    sos_handle_event(SOS_EVENT_BOOT_RUN_APPLICATION, 0);
    // assign stack pointer to stack value
    app_reset();
  } else {
    sos_handle_event(SOS_EVENT_BOOT_RUN_BOOTLOADER, 0);
    run_bootloader();
  }

  while (1)
    ;
  return 0;
}

void run_bootloader() {
  init_hw();

  led_flash_run_bootloader();

  // initialize link and run link update
  dstr("Link Start\n");
  boot_link_update((void *)sos_config.boot.link_transport_driver);
}

int check_run_app() {

  volatile u32 *software_bootloader_request =
    (u32 *)sos_config.boot.software_bootloader_request_address;

  u32 software_bootloader_request_value = *software_bootloader_request;
  *software_bootloader_request = 0;

  // check for a value program
  if ((u32)stack_ptr == 0xFFFFFFFF) {
    // code is not valid
    return 0;
  }

  // check to see if there is a software request to run the bootloader
  if (
    sos_config.boot.software_bootloader_request_value
    && sos_config.boot.software_bootloader_request_value
         == software_bootloader_request_value) {
    return 0;
  }

  // check for a board specific request (like a pin is pulled low)
  if (sos_config.boot.is_bootloader_requested()) {
    return 0;
  }

  return 1;
}

void init_hw() {
  sos_handle_event(SOS_EVENT_BOOT_RESET, 0);

#if defined DEBUG_BOOTLOADER
  dsetmode(0);
  dstr("STACK:");
  dhex((u32)stack_ptr);
  dstr("\n");
  dstr("APP:");
  dhex((u32)app_reset);
  dstr("\n");
#endif
  cortexm_delay_ms(50);
  cortexm_enable_interrupts(); // Enable the interrupts
}

/*! @} */
