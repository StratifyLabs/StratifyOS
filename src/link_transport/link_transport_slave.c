
#include "sos/link/transport.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cortexm/cortexm.h"

#include "sos/boot/boot_debug.h"
#include "sos/sos.h"

void transport_svcall_fatal(void *args) {
  CORTEXM_SVCALL_ENTER();
  sos_config.event_handler(SOS_EVENT_ROOT_FATAL, args);
}

int link_transport_slaveread(
  link_transport_driver_t *driver,
  void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context) {
  // get sos board config for link transport
  if (driver->transport_read == NULL) {
    cortexm_svcall(transport_svcall_fatal, "transport read");
  }
  return driver->transport_read(driver, buf, nbyte, callback, context);
}

int link_transport_slavewrite(
  link_transport_driver_t *driver,
  const void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context) {
  // get sos board config for link transport
  if (driver->transport_write == NULL) {
    cortexm_svcall(transport_svcall_fatal, "transport write");
  }
  return driver->transport_write(driver, buf, nbyte, callback, context);
}

u64 link_transport_gettime() {
  // the slave using blocking calls to read() so there is not need to manage timing
  return 0;
}
