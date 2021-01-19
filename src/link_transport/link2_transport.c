// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "sos/debug.h"
#include "sos/link.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMEOUT_VALUE 500

#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

void link2_transport_insert_checksum(link2_pkt_t *pkt) {
  int i;
  u16 checksum;

  // needs to be a more powerful checksum for link2 - optionally enabled

  checksum = 0;
  checksum ^= pkt->size;
  for (i = 0; i < pkt->size; i++) {
    checksum ^= pkt->data[i];
  }
  pkt->data[i] = checksum;
}

bool link2_transport_checksum_isok(link2_pkt_t *pkt) {
  u16 checksum;
  if (pkt->size <= LINK2_PACKET_DATA_SIZE) {
    checksum = pkt->data[pkt->size];
  } else {
    return false;
  }

  link2_transport_insert_checksum(pkt);
  if (checksum == pkt_checksum(pkt)) {
    return true;
  }

  return false;
}

int link2_transport_wait_start(
  link_transport_driver_t *driver,
  link2_pkt_t *pkt,
  int timeout) {
  int bytes_read;
  int count;
  count = 0;
  u64 start_time, stop_time;
  do {
    start_time = link_transport_gettime();
    bytes_read = driver->read(driver->handle, pkt, 1);
    if (bytes_read < 0) {
      return LINK_PHY_ERROR;
    }
    if (bytes_read > 0) {
      if (pkt->start == LINK2_PACKET_START) {
        return LINK2_PACKET_START;
      }
    } else {
      stop_time = link_transport_gettime();
      count += (stop_time - start_time) / 1000UL;
      if (count >= timeout) {
        return LINK_TIMEOUT_ERROR;
      }
    }
  } while (bytes_read != 1);

  return LINK_PROT_ERROR;
}

int link2_transport_wait_packet(
  link_transport_driver_t *driver,
  link2_pkt_t *pkt,
  int timeout) {
  char *p;
  int bytes;
  int count;
  int page_size;

  p = ((char *)pkt) + 1; // start received after start
  count = 0;
  bytes = 0;
  pkt->size = 0;
  u64 start_time, stop_time;
  do {
    int bytes_read;
    start_time = link_transport_gettime();

    if (bytes == 0) {
      page_size = 1;
    } else {
      page_size = (pkt->size - bytes) + LINK2_PACKET_HEADER_SIZE - 1;
    }

    bytes_read = driver->read(driver->handle, p, page_size);
    if (bytes_read < 0) {
      return LINK_PHY_ERROR;
    }

    if (bytes_read > 0) {
      if (pkt->size > LINK2_PACKET_DATA_SIZE) {
        // this is erroneous data
        return LINK_PROT_ERROR;
      }

      bytes += bytes_read;
      p += bytes_read;
      count = 0;
    } else {
      stop_time = link_transport_gettime();
      count += (stop_time - start_time) / 1000UL;
      if (count >= timeout) {
        return LINK_TIMEOUT_ERROR;
      }
    }

  } while (bytes < (pkt->size + LINK2_PACKET_HEADER_SIZE - 1));

  return 0;
}
