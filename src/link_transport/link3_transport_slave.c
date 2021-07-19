
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sos/debug.h"
#include "sos/link/transport.h"


#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int send_ack(link_transport_driver_t *driver, u8 ack, u8 checksum);

int link3_slave_start_secure_session(link_transport_driver_t *driver) {

  // send start link3
  u8 private_key[32];

  link3_pkt_auth_data_t master_info = {};

  // need to generate a random number
  link3_transport_slaveread(driver, &master_info, sizeof(master_info), NULL, NULL);

  // wait for device info
  link3_pkt_auth_data_t device_info = {};
  //driver->make_keys(private_key, device_info.public_key);

  link3_transport_slavewrite(driver, &device_info, sizeof(device_info), NULL, NULL);

  // call the auth callback with the device info

  // send the auth signature + a random number
  link3_transport_slaveread(driver, &master_info, sizeof(master_info), NULL, NULL);

  // wait for signature of random number
  //driver->sign(master_info.identifier, master_info.public_key, device_info.signature);
  link3_transport_slavewrite(driver, &device_info, sizeof(device_info), NULL, NULL);
#if 0
const int verify_result = driver->verify(device_info.identifier, device_info.signature);
#endif

  //driver->create_shared_secret(private_key, device_info.public_key);

  //check verify_result

  // the session is ready -- this last message will be encrypted
  link3_transport_slaveread(driver, &master_info, sizeof(master_info), NULL, NULL);

  return 0;
}

int link3_transport_slaveread(
  link_transport_driver_t *driver,
  void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context) {
  u16 checksum;

  link3_pkt_t pkt = {};
  int bytes = 0;
  char * p = buf;

  link3_pkt_data_t * const data = (link3_pkt_data_t *)pkt.data;

  do {

    if (link3_transport_wait_start(driver, &pkt, driver->timeout) < 0) {
      driver->flush(driver->handle);
      send_ack(driver, LINK3_PACKET_NACK, 0);
      return -1 * __LINE__;
    }

    if (link3_transport_wait_packet(driver, &pkt, driver->timeout) < 0) {
      driver->flush(driver->handle);
      send_ack(driver, LINK3_PACKET_NACK, 0);
      return -1 * __LINE__;
    }

    if (pkt.start != LINK3_PACKET_START) {
      // if packet does not start with the start byte then it is not a packet
      driver->flush(driver->handle);
      send_ack(driver, LINK3_PACKET_NACK, 0);
      return -1 * __LINE__;
    }

    // a packet has arrived -- checksum it
    if (driver->o_flags & LINK3_FLAG_IS_CHECKSUM) {
      checksum = pkt_checksum(&pkt);
      if (link3_transport_checksum_isok(&pkt) == false) {
        // bad checksum on packet -- treat as a non-packet
        driver->flush(driver->handle);
        send_ack(driver, LINK3_PACKET_NACK, checksum);
        return -1 * __LINE__;
      }
    } else {
      checksum = 0;
    }

    const u16 unaligned_bytes = pkt.size % 16;
    const u16 padding_bytes = unaligned_bytes ? 16 - unaligned_bytes : 0;
    memset(data->data + data->data_size, 0, padding_bytes);

    // callback to handle incoming data as it arrives
    if (callback == NULL) {
      // copy the valid data to the buffer

      //decrypt?

      memcpy(p, data->data, data->data_size);
      bytes += data->data_size;
      p += data->data_size;
      send_ack(driver, LINK3_PACKET_ACK, checksum);
    } else {
      int result;

      //decrypt??

      if ((result = callback(context, pkt.data, pkt.size)) < 0) {
        send_ack(driver, LINK3_PACKET_NACK, checksum);
        return result;
      } else {
        bytes += data->data_size;
        if (send_ack(driver, LINK3_PACKET_ACK, checksum) < 0) {
          return -1 * __LINE__;
        }
      }
    }

  } while ((bytes < nbyte) && (data->data_size == sizeof(data->data)));

  if (bytes == 0) {
    driver->flush(driver->handle);
  }

  return bytes;
}

int link3_transport_slavewrite(
  link_transport_driver_t *driver,
  const void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context) {
  int ret = 0;
  link3_pkt_t pkt = {};

  int bytes = 0;
  char* p = (void *)buf;
  pkt.start = LINK3_PACKET_START;
  pkt.o_flags = driver->o_flags;

  link3_pkt_data_t * const data = (link3_pkt_data_t *)pkt.data;

  do {

    if ((nbyte - bytes) > (int)sizeof(data->data)) {
      data->data_size = sizeof(data->data);
    } else {
      data->data_size = nbyte - bytes;
    }

    //encrypt?

    if (callback != NULL) {
      if ((ret = callback(context, pkt.data, pkt.size)) < 0) {
        // could not get the desired data
        pkt.size = 0;
      } else {
        pkt.size = ret;
      }
    } else {
      // copy data from buf
      memcpy(pkt.data, p, data->data_size);
    }

    if (driver->o_flags & LINK3_FLAG_IS_CHECKSUM) {
      link3_transport_insert_checksum(&pkt);
    }

    // send packet
    if (
      driver->write(driver->handle, &pkt, pkt.size + LINK3_PACKET_HEADER_SIZE)
      != (pkt.size + LINK3_PACKET_HEADER_SIZE)) {
      return -1 * __LINE__;
    }

    bytes += pkt.size;
    p += pkt.size;

  } while ((bytes < nbyte) && (data->data_size == sizeof(data->data)));

  if (callback && (bytes == 0)) {
    bytes = ret;
  }

  return bytes;
}

int send_ack(link_transport_driver_t *driver, u8 ack, u8 checksum) {
  link_ack_t ack_pkt;
  ack_pkt.ack = ack;
  ack_pkt.checksum = checksum;
  return driver->write(driver->handle, &ack_pkt, sizeof(ack_pkt));
}
