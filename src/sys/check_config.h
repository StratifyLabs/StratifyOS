#ifndef CHECK_CONFIG_H
#define CHECK_CONFIG_H

#define CHECK_CONFIG_MESSAGE(x, y) ((void *)((MCU_STRINGIFY(x) "." MCU_STRINGIFY(y))))

#define CHECK_CONFIG(GROUP_VALUE, MEMBER_VALUE)                                          \
  if (sos_config.GROUP_VALUE.MEMBER_VALUE == 0)                                          \
    sos_handle_event(                                                                    \
      SOS_EVENT_ROOT_FATAL, CHECK_CONFIG_MESSAGE(GROUP_VALUE, MEMBER_VALUE));

#define CONFIG_ASSERT(value, message)                                                    \
  if ((value) == 0)                                                                        \
    sos_handle_event(SOS_EVENT_ROOT_FATAL, message);

#endif // CHECK_CONFIG_H
