#ifndef CHECK_CONFIG_H
#define CHECK_CONFIG_H

#define CHECK_SLEEP_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.sleep.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_SLEEP_CONFIG, (void *)&sos_config.sleep.MEMBER_VALUE);

#define CHECK_CACHE_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.cache.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_CACHE_CONFIG, (void *)&sos_config.cache.MEMBER_VALUE);

#define CHECK_USB_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.usb.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_USB_CONFIG, (void *)&sos_config.usb.MEMBER_VALUE);

#define CHECK_TASK_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.task.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_TASK_CONFIG, (void *)&sos_config.task.MEMBER_VALUE);

#define CHECK_CLOCK_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.clock.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_CLOCK_CONFIG, (void *)&sos_config.clock.MEMBER_VALUE);

#define CHECK_SYS_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.sys.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_SYS_CONFIG, (void *)&sos_config.sys.MEMBER_VALUE);

#define CHECK_MCU_CONFIG(MEMBER_VALUE)                                                 \
  if (sos_config.mcu.MEMBER_VALUE == NULL)                                             \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_PANIC_MCU_CONFIG, (void *)&sos_config.mcu.MEMBER_VALUE);


#endif // CHECK_CONFIG_H
