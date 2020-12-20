// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <stdbool.h>

#include "link_local.h"

int link_open_stdio(link_transport_mdriver_t *driver /*, const char * name */) {
  return 0;
}

int link_close_stdio(link_transport_mdriver_t *driver) { return 0; }

int link_read_stdout(link_transport_mdriver_t *driver, void *buf, int nbyte) {
  return driver->phy_driver.read(driver->phy_driver.handle, buf, nbyte);
}

int link_write_stdin(link_transport_mdriver_t *driver, const void *buf, int nbyte) {
  return driver->phy_driver.write(driver->phy_driver.handle, buf, nbyte);
}
