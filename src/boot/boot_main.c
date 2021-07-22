
#include <sys/lock.h>

#include "device/auth.h"

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

#include "sos_config.h"

static const u8 *get_public_key() {
  return (const u8 *)((u32)sos_config.sys.secret_key_address & ~0x01);
}

void boot_invoke_bootloader(void *args) {
  MCU_UNUSED_ARGUMENT(args);
  // write SW location with key and then reset
  u32 *bootloader_software_request_address =
    (u32 *)sos_config.boot.software_bootloader_request_address;
  *bootloader_software_request_address =
    sos_config.boot.software_bootloader_request_value;

  sos_config.cache.clean_data();
  cortexm_reset(0);
}

void sos_handle_event(int event, void *args) { sos_config.event_handler(event, args); }

extern u32 _etext;

void init_hw();

static void *stack_ptr;
static void (*app_reset)();

void run_bootloader();

void led_flash() {
  while (1) {
    sos_config.debug.enable_led();
    cortexm_delay_ms(500);
    sos_config.debug.disable_led();
    cortexm_delay_ms(500);
  }
}

void led_flash_error() {
  while (1) {
    sos_config.debug.enable_led();
    cortexm_delay_ms(50);
    sos_config.debug.disable_led();
    cortexm_delay_ms(50);
  }
}

void led_flash_run_bootloader() {
  int i;
  for (i = 0; i < 3; i++) {
    sos_config.debug.enable_led();
    cortexm_delay_ms(50);
    sos_config.debug.disable_led();
    cortexm_delay_ms(50);
  }
}

static int check_run_app();
void run_bootloader();

/*! \details
 */
void boot_main() {
  stack_ptr = (void *)(((u32 *)sos_config.boot.program_start_address)[0]);
  app_reset = (void (*)())((((u32 *)sos_config.boot.program_start_address)[1]));

  if (check_run_app()) {
    // led_flash_error();
    sos_handle_event(SOS_EVENT_BOOT_RUN_APPLICATION, 0);
    // assign stack pointer to stack value
    app_reset();
  } else {
    sos_handle_event(SOS_EVENT_BOOT_RUN_BOOTLOADER, 0);
    run_bootloader();
  }

  while (1)
    ;
}

void run_bootloader() {
  init_hw();

  led_flash_run_bootloader();

  // initialize link and run link update
  dstr("Link Start-\n");
  boot_link_update((void *)sos_config.boot.link_transport_driver);
}

int check_run_app() {
  // check for a value program
  if ((u32)stack_ptr == 0xFFFFFFFF) {
    // code is not valid
    return 0;
  }

  // check for a board specific request (like a pin is pulled low)
  if (sos_config.boot.is_bootloader_requested()) {
    return 0;
  }

  volatile u32 *software_bootloader_request =
    (u32 *)sos_config.boot.software_bootloader_request_address;

  u32 software_bootloader_request_value = *software_bootloader_request;
  *software_bootloader_request = 0;

  // check to see if there is a software request to run the bootloader
  if (
    sos_config.boot.software_bootloader_request_value
    && sos_config.boot.software_bootloader_request_value
         == software_bootloader_request_value) {
    return 0;
  }

  return 1;
}

int boot_handle_auth_event(int event, void *args) {
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
  if (event == BOOTLOADER_EVENT_AUTHENTICATE) {
    bootloader_event_authenication_t *auth = args;
    auth_token_t input;
    auth_token_t result;
    memcpy(input.data, auth->auth_data, 32);
    const int auth_result =
      auth_pure_code_calculate_authentication(&result, &input, auth->is_key_first);
    if (auth_result < 0) {
      return auth_result;
    }

    memcpy(auth->result, result.data, 32);
    return 0;
  }

  if (event == BOOTLOADER_EVENT_GET_PUBLIC_KEY) {
    bootloader_public_key_t *key = args;
    memcpy(key, get_public_key, sizeof(bootloader_public_key_t));
    return 0;
  }
#endif

#if CONFIG_BOOT_IS_AES_CRYPTO
  if (event == BOOTLOADER_EVENT_ENCRYPT) {
    bootloader_event_crypto_t *crypto = args;
    const int crypto_result = auth_pure_code_encrypt_decrypt(
      crypto->iv, crypto->plain, crypto->cipher, crypto->nbyte, AUTH_PURE_CODE_IS_ENCRYPT);
    return crypto_result;
  }

  if (event == BOOTLOADER_EVENT_DECRYPT) {
    bootloader_event_crypto_t *crypto = args;
    const int crypto_result = auth_pure_code_encrypt_decrypt(
      crypto->iv, crypto->plain, crypto->cipher, crypto->nbyte, AUTH_PURE_CODE_IS_DECRYPT);
    return crypto_result;
  }
#endif

  return 0;
}

void init_hw() {
  sos_handle_event(SOS_EVENT_BOOT_RESET, 0);
  sos_config.sys.initialize();
  sos_config.debug.initialize();

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
