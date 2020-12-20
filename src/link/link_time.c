// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "link_local.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int link_settime(link_transport_mdriver_t *driver, struct link_tm *t) {
  int fd;
  int ret;
  struct link_tm ltm;

  memcpy(&ltm, t, sizeof(struct link_tm));

  link_debug(LINK_DEBUG_MESSAGE, "open rtc device");
  fd = link_open(driver, "/dev/rtc", LINK_O_RDWR);
  if (fd < 0) {
    return -1;
  }

  link_debug(LINK_DEBUG_MESSAGE, "write time");
  ret = link_ioctl(driver, fd, I_RTC_SET, &ltm);

  link_debug(LINK_DEBUG_MESSAGE, "close");
  if (link_close(driver, fd) < 0) {
    return -1;
  }

  return ret;
}

int link_gettime(link_transport_mdriver_t *driver, struct link_tm *t) {
  int fd;
  int ret;

  link_debug(LINK_DEBUG_MESSAGE, "Open RTC fildes");
  fd = link_open(driver, "/dev/rtc", LINK_O_RDWR);
  if (fd < 0) {
    return -1;
  }

  ret = link_ioctl(driver, fd, I_RTC_GET, t);
  if (ret < 0) {
    link_error("Failed to I_RTC_GET");
  }

  link_debug(LINK_DEBUG_MESSAGE, "Close RTC fildes");
  if (link_close(driver, fd) < 0) {
    link_error("failed to close");
    return -1;
  }

  return ret;
}

/*! @} */
