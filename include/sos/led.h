#ifndef SOS_LED_H
#define SOS_LED_H

#ifdef __cplusplus
extern "C" {
#endif

void sos_led_startup();
void sos_led_boot_startup();
void sos_led_flash(int count);
void sos_led_svcall_enable(void *args);
void sos_led_svcall_disable(void *args);
void sos_led_svcall_error(void *args);

void sos_led_root_enable();
void sos_led_root_disable();
void sos_led_root_error();

#ifdef __cplusplus
}
#endif

#endif // SOS_LED_H
