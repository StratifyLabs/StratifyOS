// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEV_LINK_TRANSPORT_H_
#define DEV_LINK_TRANSPORT_H_

#include <sdk/api.h>

#define LINK_MAX_TRANSFER_SIZE 64
#define LINK_MAX_PACKET_SIZE (63)
#define LINK_PACKET_HEADER_SIZE (3) // start, size and checksum (1 byte)
#define LINK_PACKET_DATA_SIZE (LINK_MAX_PACKET_SIZE - LINK_PACKET_HEADER_SIZE)

#define LINK_PACKET_STDIO_FLAG (1 << 15)

#define LINK_PACKET_START (16) // DLE character
#define LINK_PACKET_ACK (0x06)
#define LINK_PACKET_NACK (0x53)

#define LINK2_PACKET_START (17)
#define LINK2_PACKET_HEADER_SIZE (6) // start, size and checksum (2 bytes)
#define LINK2_MAX_PACKET_SIZE (1024 + LINK2_PACKET_HEADER_SIZE)
#define LINK2_PACKET_DATA_SIZE (LINK2_MAX_PACKET_SIZE - LINK2_PACKET_HEADER_SIZE)
#define LINK2_PACKET_ACK (0x07)
#define LINK2_PACKET_NACK (0x54)

#define LINK3_PACKET_START (18)
#define LINK3_PACKET_HEADER_SIZE (6) // start, size, and checksum (2 bytes)
#define LINK3_PACKET_DATA_SIZE (LINK3_MAX_PACKET_SIZE - LINK3_PACKET_HEADER_SIZE)
#define LINK3_PACKET_PAYLOAD_SIZE (992)


#define LINK3_PACKET_ACK (0x08)
#define LINK3_PACKET_NACK (0x55)

enum link2_flags { LINK2_FLAG_IS_CHECKSUM = (1 << 0) };
enum link3_flags { LINK3_FLAG_IS_CHECKSUM = (1 << 0) };

typedef struct MCU_PACK {
  u8 ack;
  u8 checksum;
} link_ack_t;

typedef struct MCU_PACK {
  u8 start;
  u8 size;
  u8 data[LINK_PACKET_DATA_SIZE + 1];
} link_pkt_t;

typedef struct MCU_PACK {
  u8 start;
  u8 o_flags;
  u16 size;
  u8 data[LINK2_PACKET_DATA_SIZE + 2]; // 2 checksum bytes
} link2_pkt_t;

#define LINK3_STATE_OPEN 0
#define LINK3_STATE_MASTER_INFO 1
#define LINK3_STATE_DEVICE_INFO 2
#define LINK3_STATE_MASTER_AUTH 3
#define LINK3_STATE_DEVICE_AUTH 4
#define LINK3_STATE_SECURE 5


// generic link3 packet
typedef struct MCU_PACK {
  u32 data_size;
  u8 iv[16];
  u8 data[LINK3_PACKET_PAYLOAD_SIZE];
} link3_pkt_data_t;

#define LINK3_MAX_PACKET_SIZE (sizeof(link3_pkt_data_t) + 2)

typedef struct MCU_PACK {
  u8 start;
  u8 o_flags;
  u16 size;
  u8 data[LINK3_MAX_PACKET_SIZE]; // 2 checksum bytes
} link3_pkt_t;


//sizeof link3_pkt_data_t.data must be divisible by 16

typedef struct MCU_PACK {
  u8 identifier[32];
  u8 public_key[64];
  u8 signature[64];
} link3_pkt_auth_data_t;

#if defined __link
typedef void *link_transport_phy_t;
#if defined __cplusplus
#define LINK_PHY_OPEN_ERROR (static_cast<void *>(nullptr))
#else
#define LINK_PHY_OPEN_ERROR ((void *)0)
#endif
#else
typedef int link_transport_phy_t;
#define LINK_PHY_OPEN_ERROR (-1)

typedef struct {
  const void *buf;
  int nbyte;
} link_transport_notify_t;

#endif

typedef struct {
  int baudrate;
  int stop_bits;
  int parity;
} link_transport_serial_options_t;

typedef struct  {

} link_transport_crypto_driver_t;

typedef struct  {
  void * ecc_context;
  const crypt_ecc_api_t * ecc_api;
  void * aes_context;
  const crypt_aes_api_t * aes_api;
  void * random_context;
  const crypt_random_api_t * random_api;
} link_transport_crypto_handle_t;

typedef struct link_transport_driver {
  link_transport_phy_t handle;
  link_transport_phy_t (*open)(const char *, const void *options);
  int (*write)(link_transport_phy_t, const void *, int);
  int (*read)(link_transport_phy_t, void *, int);
  int (*close)(link_transport_phy_t *);
  void (*wait)(int);
  void (*flush)(link_transport_phy_t);
  void (*request)(link_transport_phy_t);
  int (*transport_read)(
    struct link_transport_driver *driver,
    void *buf,
    int nbyte,
    int (*callback)(void *, void *, int),
    void *context);
  int (*transport_write)(
    struct link_transport_driver *driver,
    const void *buf,
    int nbyte,
    int (*callback)(void *, void *, int),
    void *context);
  int timeout;
  u8 o_flags;
  u8 shared_secret[32];
  const link_transport_crypto_handle_t * crypto_handle;
} link_transport_driver_t;

typedef struct {
  int (*getname)(char *dest, const char *last, int len);
  int (*lock)(link_transport_phy_t handle);
  int (*unlock)(link_transport_phy_t handle);
  int (*status)(link_transport_phy_t handle);
  link_transport_driver_t phy_driver;
  char dev_name[64];
  char notify_name[64];
  const void *options;
  u32 transport_version; // which version of the protocol is the slave running
  u16 path_max;
  u16 arg_max;

  // link3 session parameters
  int (*sign)(
    const u8 identifier[32],
    const u8 random_number[64],
    u8 signature[64]);
  int (*verify)(
    const u8 identifier[32],
    const u8 signature[64]
    );
} link_transport_mdriver_t;

typedef struct {
  void (*set_timeout)(link_transport_mdriver_t *driver, int t);
  int (*write)(link_transport_mdriver_t *driver, const void *buf, int nbyte);
  int (*read)(link_transport_mdriver_t *driver, void *buf, int nbyte);
} link_transport_master_t;

typedef struct {
  int (*write)(
    link_transport_driver_t *driver,
    const void *buf,
    int nbyte,
    int (*callback)(void *, void *, int),
    void *context);
  int (*read)(
    link_transport_driver_t *driver,
    void *buf,
    int nbyte,
    int (*callback)(void *, void *, int),
    void *context);
} link_transport_slave_t;

#if defined __cplusplus
extern "C" {
#endif

void link_transport_mastersettimeout(link_transport_mdriver_t *driver, int t);
int link_transport_masterwrite(
  link_transport_mdriver_t *driver,
  const void *buf,
  int nbyte);
int link_transport_masterread(link_transport_mdriver_t *driver, void *buf, int nbyte);

int link_transport_slavewrite(
  link_transport_driver_t *driver,
  const void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);
int link_transport_slaveread(
  link_transport_driver_t *driver,
  void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);

u64 link_transport_gettime();

void link1_transport_mastersettimeout(link_transport_mdriver_t *driver, int t);
int link1_transport_masterwrite(
  link_transport_mdriver_t *driver,
  const void *buf,
  int nbyte);
int link1_transport_masterread(link_transport_mdriver_t *driver, void *buf, int nbyte);
int link1_transport_slavewrite(
  link_transport_driver_t *driver,
  const void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);
int link1_transport_slaveread(
  link_transport_driver_t *driver,
  void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);

void link1_transport_insert_checksum(link_pkt_t *pkt);
bool link1_transport_checksum_isok(link_pkt_t *pkt);
int link1_transport_wait_packet(
  link_transport_driver_t *driver,
  link_pkt_t *pkt,
  int timeout);
int link1_transport_wait_start(
  link_transport_driver_t *driver,
  link_pkt_t *pkt,
  int timeout);

void link2_transport_mastersettimeout(link_transport_mdriver_t *driver, int t);
int link2_transport_masterwrite(
  link_transport_mdriver_t *driver,
  const void *buf,
  int nbyte);
int link2_transport_masterread(link_transport_mdriver_t *driver, void *buf, int nbyte);
int link2_transport_slavewrite(
  link_transport_driver_t *driver,
  const void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);
int link2_transport_slaveread(
  link_transport_driver_t *driver,
  void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);
void link2_transport_insert_checksum(link2_pkt_t *pkt);
bool link2_transport_checksum_isok(link2_pkt_t *pkt);
int link2_transport_wait_packet(
  link_transport_driver_t *driver,
  link2_pkt_t *pkt,
  int timeout);
int link2_transport_wait_start(
  link_transport_driver_t *driver,
  link2_pkt_t *pkt,
  int timeout);

void link3_transport_mastersettimeout(link_transport_mdriver_t *driver, int t);
int link3_transport_masterwrite(
  link_transport_mdriver_t *driver,
  const void *buf,
  int nbyte);
int link3_transport_masterread(link_transport_mdriver_t *driver, void *buf, int nbyte);
int link3_transport_slavewrite(
  link_transport_driver_t *driver,
  const void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);
int link3_transport_slaveread(
  link_transport_driver_t *driver,
  void *buf,
  int nbyte,
  int (*callback)(void *, void *, int),
  void *context);
void link3_transport_insert_checksum(link3_pkt_t *pkt);
bool link3_transport_checksum_isok(link3_pkt_t *pkt);
int link3_transport_wait_packet(
  link_transport_driver_t *driver,
  link3_pkt_t *pkt,
  int timeout);
int link3_transport_wait_start(
  link_transport_driver_t *driver,
  link3_pkt_t *pkt,
  int timeout);

int link3_start_secure_session(link_transport_mdriver_t *driver);

#if defined __cplusplus
}
#endif

#endif /* DEV_LINK_TRANSPORT_H_ */
