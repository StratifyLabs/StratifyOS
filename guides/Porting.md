# Porting Guide (experimental)

Stratify OS is an operating system specifically designed for the ARM Cortex-M architecture. It supports the following architectures:

- ARM Cortex M3 (armv7m)
- ARM Cortex M4 (armv7em)
- ARM Cortex M4F (armv7em_f4sh)
- ARM Cortex M7 (armv7em_f5sh and armv7em_f5dh)

There is planned support for the ARM Cortex M23 and M33 architectures though no chips are currently supported.

# Configuration

Stratify OS is ported using link level configuration. Your board support package must provide a single variable populated with constant data.

```c++
typedef struct MCU_PACK {
  sos_sys_config_t sys;
  sos_fs_config_t fs;
  sos_clock_config_t clock;
  sos_task_config_t task;
  sos_debug_config_t debug;
  sos_cache_config_t cache;
  sos_sleep_config_t sleep;
  sos_usb_config_t usb;
  const sos_socket_api_t *socket_api;
  void (*event_handler)(int, void *);
} sos_config_t;
```

The details of the `sos_config_t` are listed in the [sos/config.h](../include/sos/config.h) header file.




