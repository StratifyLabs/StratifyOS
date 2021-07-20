// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "sos/fs/sysfs.h"
#include "sos/link.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __win32
#define DEFAULT_TIMEOUT_VALUE 500
#else
#define DEFAULT_TIMEOUT_VALUE 500
#endif

#define pkt_checksum(pktp) ((pktp)->data[(pktp)->size])

static int wait_ack(link_transport_mdriver_t *driver, u8 checksum, int timeout);

static void *ecc_context(link_transport_mdriver_t *driver) {
  return driver->phy_driver.crypto_handle.ecc_context;
}

static const crypt_ecc_api_t *ecc_api(link_transport_mdriver_t *driver) {
  return driver->phy_driver.crypto_driver->ecc_api;
}

static void *aes_context(link_transport_mdriver_t *driver) {
  return driver->phy_driver.crypto_handle.aes_context;
}

static const crypt_aes_api_t *aes_api(link_transport_mdriver_t *driver) {
  return driver->phy_driver.crypto_driver->aes_api;
}

static void *random_context(link_transport_mdriver_t *driver) {
  return driver->phy_driver.crypto_handle.random_context;
}

static const crypt_random_api_t *random_api(link_transport_mdriver_t *driver) {
  return driver->phy_driver.crypto_driver->random_api;
}

int link3_start_secure_session(link_transport_mdriver_t *driver) {

  // deinit if needed
  ecc_api(driver)->deinit(&(driver->phy_driver.crypto_handle.ecc_context));
  aes_api(driver)->deinit(&(driver->phy_driver.crypto_handle.aes_context));
  random_api(driver)->deinit(&(driver->phy_driver.crypto_handle.random_context));

  // send start link3
  random_api(driver)->init(&(driver->phy_driver.crypto_handle.random_context));
  ecc_api(driver)->init(&(driver->phy_driver.crypto_handle.ecc_context));

  // create a key pair, the device will sign the public key with it's secret, private key
  link3_pkt_auth_data_t master_info = {};
  u32 master_key_size = sizeof(master_info.public_key);
  ecc_api(driver)->dh_create_key_pair(
    ecc_context(driver), CRYPT_ECC_KEY_PAIR_SECP256R1, master_info.public_key,
    &master_key_size);
  link3_transport_masterwrite(driver, &master_info, sizeof(master_info));

  // wait for device info
  link3_pkt_auth_data_t device_info = {};
  link3_transport_masterread(driver, &device_info, sizeof(device_info));

  link_transport_device_keys_t device_keys;

  const int get_device_keys_result =
    driver->get_device_keys(device_info.identifier, &device_keys);

  if (get_device_keys_result < 0) {
    // send zero filled packet to indicate failure
    master_info = (link3_pkt_auth_data_t){};
    link3_transport_masterwrite(driver, &master_info, sizeof(master_info));
    return -1;
  }

  u32 signature_size = sizeof(master_info.signature);
  // need to fetch the device's private key to create the signature
  void *ecc_sign_context = NULL;

  ecc_api(driver)->init(&ecc_sign_context);

  ecc_api(driver)->dsa_set_key_pair(
    ecc_sign_context, device_keys.private_key, 32, device_keys.public_key, 64);
  const int dsa_sign_result = ecc_api(driver)->dsa_sign(
    ecc_sign_context, device_info.public_key, sizeof(device_info.public_key),
    master_info.signature, &signature_size);

  if (dsa_sign_result < 0) {
    // send zero filled packet to indicate failure
    master_info = (link3_pkt_auth_data_t){};
    link3_transport_masterwrite(driver, &master_info, sizeof(master_info));
    return -1;
  }

  // send the auth signature + a random number
  link3_transport_masterwrite(driver, &master_info, sizeof(master_info));

  // wait for signature of random number
  link3_transport_masterread(driver, &device_info, sizeof(device_info));

  const int verify_device_signature_result = ecc_api(driver)->dsa_verify(
    ecc_sign_context, device_info.signature, 64, device_keys.public_key, 64);

  ecc_api(driver)->deinit(&ecc_sign_context);

  ecc_api(driver)->dh_calculate_shared_secret(
    ecc_context(driver), device_info.public_key, sizeof(device_info.public_key),
    driver->phy_driver.shared_secret, sizeof(driver->phy_driver.shared_secret));

  if (verify_device_signature_result != 1) {
    master_info = (link3_pkt_auth_data_t){};
    link3_transport_masterwrite(driver, &master_info, sizeof(master_info));
    return -1;
  }

  // the session is ready -- messages after this will be encrypted
  link3_transport_masterwrite(driver, &master_info, sizeof(master_info));

  driver->transport_version = 3;

  aes_api(driver)->init(&(driver->phy_driver.crypto_handle.aes_context));
  aes_api(driver)->set_key(aes_context(driver), driver->phy_driver.shared_secret, 128, 8);

  return 0;
}

void link3_transport_mastersettimeout(link_transport_mdriver_t *driver, int t) {
  if (t == 0) {
    driver->phy_driver.timeout = DEFAULT_TIMEOUT_VALUE;
  } else {
    driver->phy_driver.timeout = t;
  }
}

int link3_transport_masterread(link_transport_mdriver_t *driver, void *buf, int nbyte) {
  link3_pkt_t pkt;
  int err;

  int bytes = 0;
  u8 *p = buf;

  link3_pkt_data_t *const data = (link3_pkt_data_t *)pkt.data;

  do {

    if (
      (err = link3_transport_wait_start(
         &driver->phy_driver, &pkt, driver->phy_driver.timeout))
      < 0) {
      // printf("\nerror %s():%d result:%d\n", __FUNCTION__, __LINE__, err);
      driver->phy_driver.flush(driver->phy_driver.handle);
      return err;
    }

    if (
      (err = link3_transport_wait_packet(
         &driver->phy_driver, &pkt, driver->phy_driver.timeout))
      < 0) {
      // printf("\nerror %s():%d result:%d\n", __FUNCTION__, __LINE__, err);
      driver->phy_driver.flush(driver->phy_driver.handle);
      return err;
    }

    if (driver->phy_driver.o_flags & LINK3_FLAG_IS_CHECKSUM) {
      // a packet has arrived -- checksum it
      if (link3_transport_checksum_isok(&pkt) == false) {
        return SYSFS_SET_RETURN(1);
      }
    }

    // callback to handle incoming data as it arrives
    // copy the valid data to the buffer
    if (data->data_size + bytes > (u32)nbyte) {
      // if the target device has a bug, this will prevent a seg fault
      data->data_size = nbyte - bytes;
    }

    if (driver->transport_version == 3) {
      // decrypt the packet
      const u16 unaligned_bytes = pkt.size % 16;
      const u16 padding_bytes = unaligned_bytes ? 16 - unaligned_bytes : 0;
      memset(data->data + data->data_size, 0, padding_bytes);
      aes_api(driver)->decrypt_cbc(
        aes_context(driver), data->data_size + padding_bytes, data->iv, data->data, p);

    } else {
      memcpy(p, pkt.data, data->data_size);
    }

    bytes += data->data_size;
    p += data->data_size;

  } while ((bytes < nbyte) && (data->data_size == sizeof(data->data)));

  return bytes;
}

int link3_transport_masterwrite(
  link_transport_mdriver_t *driver,
  const void *buf,
  int nbyte) {
  int bytes;
  int err;

  if (driver == 0) {
    return -1;
  }

  bytes = 0;
  const u8 * p = buf;
  link3_pkt_t pkt = {.start = LINK3_PACKET_START, .o_flags = driver->phy_driver.o_flags};
  link3_pkt_data_t *data = (link3_pkt_data_t *)pkt.data;

  do {

    if ((nbyte - bytes) > (int)sizeof(data->data)) {
      data->data_size = sizeof(data->data);
    } else {
      data->data_size = nbyte - bytes;
    }

    // total packet size -- data size plus header
    pkt.size = data->data_size + (sizeof(*data) - sizeof(data->data));

    if (driver->transport_version == 3) {
      // this is the actual number of data bytes before padding
      const u16 unaligned_bytes = pkt.size % 16;
      const u16 padding_bytes = unaligned_bytes ? 16 - unaligned_bytes : 0;
      memset(data->data + data->data_size, 0, padding_bytes);

      random_api(driver)->random(random_context(driver), data->iv, sizeof(data->iv));

      aes_api(driver)->encrypt_cbc(
        aes_context(driver), data->data_size + padding_bytes, data->iv, p, data->data);

    } else {
      memcpy(data->data, p, data->data_size);
    }

    if (driver->phy_driver.o_flags & LINK3_FLAG_IS_CHECKSUM) {
      link3_transport_insert_checksum(&pkt);
    } else {
      // checksum is set to zero
      pkt_checksum(&pkt) = 0;
    }

    // send packet
    if (
      driver->phy_driver.write(
        driver->phy_driver.handle, &pkt, pkt.size + LINK3_PACKET_HEADER_SIZE)
      != (pkt.size + LINK3_PACKET_HEADER_SIZE)) {
      return SYSFS_SET_RETURN(1);
    }

    // received ack of the checksum
    if ((err = wait_ack(driver, pkt_checksum(&pkt), driver->phy_driver.timeout)) < 0) {
      driver->phy_driver.flush(driver->phy_driver.handle);
#if 0
      printf(
        "\nerror %s():%d 0x%X-%d (%d)\n", __FUNCTION__, __LINE__, err, err,
        driver->phy_driver.timeout);
#endif
      return err;
    }

    if (err != LINK3_PACKET_ACK) {
      return SYSFS_SET_RETURN(1);
    }

    bytes += data->data_size;
    p += data->data_size;

  } while ((bytes < nbyte) && (data->data_size == sizeof(data->data)));

  return bytes;
}

int wait_ack(link_transport_mdriver_t *driver, u8 checksum, int timeout) {
  link_ack_t ack;
  int ret;

  int count = 0;
  char *p = (char *)&ack;
  size_t bytes_read = 0;
  u64 start_time, stop_time;
  do {
    start_time = link_transport_gettime();
    ret = driver->phy_driver.read(driver->phy_driver.handle, p, sizeof(ack) - bytes_read);

    if (ret < 0) {
      return LINK_PHY_ERROR;
    }

    if (ret > 0) {
      bytes_read += ret;
      p += ret;
      count = 0;
    } else {
      stop_time = link_transport_gettime();
      count += (stop_time - start_time) / 1000UL;
      if (count >= timeout) {
        return LINK_TIMEOUT_ERROR;
      }
    }
  } while (bytes_read < sizeof(ack));

  if (ack.checksum != checksum) {
    return LINK_PROT_ERROR;
  }

  return ack.ack;
}
