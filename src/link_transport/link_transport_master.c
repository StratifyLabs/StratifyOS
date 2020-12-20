// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "../link/link_local.h"
#include "sos/fs/sysfs.h"
#include "sos/link.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __win32
#define TIMEOUT_VALUE 500
#else
#define INITIAL_TIMEOUT_VALUE 500 // used for first ping
#define TIMEOUT_VALUE 5000        // used for more complex operations
#endif

#define INITIAL_TIMEOUT_VALUE 500 // used for first ping

#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int wait_ack(link_transport_mdriver_t *driver, uint8_t checksum, int timeout);
static int m_timeout_value = TIMEOUT_VALUE;
static int resolve_protocol(link_transport_mdriver_t *driver);

void link_transport_mastersettimeout(link_transport_mdriver_t *driver, int t) {
  if (resolve_protocol(driver) < 0) {
    link_error("failed to resolve protocol");
    return;
  }

  if (driver->transport_version == 1) {
    return link1_transport_mastersettimeout(driver, t);
  }

  if (driver->transport_version == 2) {
    return link2_transport_mastersettimeout(driver, t);
  }
}

int link_transport_masterread(link_transport_mdriver_t *driver, void *buf, int nbyte) {
  int result;
  if ((result = resolve_protocol(driver)) < 0) {
    link_error("failed to resolve protocol with %d", result);
    return result;
  }

  if (driver->transport_version == 1) {
    return link1_transport_masterread(driver, buf, nbyte);
  }

  if (driver->transport_version == 2) {
    return link2_transport_masterread(driver, buf, nbyte);
  }

  link_error("tranport version is an invalid value (%d)", driver->transport_version);
  return LINK_PROT_ERROR;
}

int link_transport_masterwrite(
  link_transport_mdriver_t *driver,
  const void *buf,
  int nbyte) {
  int result;
  if ((result = resolve_protocol(driver)) < 0) {
    link_error("failed to resolve protocol with %d", result);
    return result;
  }

  if (driver->transport_version == 1) {
    return link1_transport_masterwrite(driver, buf, nbyte);
  }

  if (driver->transport_version == 2) {
    return link2_transport_masterwrite(driver, buf, nbyte);
  }

  link_error("tranport version is an invalid value (%d)", driver->transport_version);
  return LINK_PROT_ERROR;
}

int resolve_protocol(link_transport_mdriver_t *driver) {

  if ((driver == 0) || (driver->phy_driver.handle == 0)) {
    link_debug(LINK_DEBUG_WARNING, "driver is null -- phy error");
    return LINK_PHY_ERROR;
  }

  if (driver->transport_version == 0) {
    // need to do protocol resolution starting with link1
    int result = link1_transport_masterwrite(driver, 0, 0);
    if (result == 0) {
      // printf("------------------- Resolved to Link1 -------------------\n");
      driver->transport_version = 1;
    } else {
      int nack = SYSFS_GET_RETURN_ERRNO(result);
      if (nack == LINK2_PACKET_NACK) {
        // printf("------------------- Resolved to Link2 -------------------\n");
        driver->transport_version = 2;
      } else {
        // printf("------------------- Not Resolved -------------------\n");
        return LINK_PROT_ERROR;
      }
    }
  }

  if (driver->transport_version > 2) {
    driver->transport_version = 0;
    return LINK_PROT_ERROR;
  }

  return driver->transport_version;
}

u64 link_transport_gettime() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  u64 result = (u64)ts.tv_sec * 1000000UL + (u64)ts.tv_nsec / 1000UL;
  return result;
}
