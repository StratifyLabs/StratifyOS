// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_API_WIFI_API_H
#define SOS_API_WIFI_API_H

#include <sdk/api.h>

#include <sos/dev/netif.h>

enum {
  WIFI_SECURITY_INVALID,
  WIFI_SECURITY_OPEN,
  WIFI_SECURITY_WEP,
  WIFI_SECURITY_WPA_PSK,
  WIFI_SECURITY_802_1X,
};

enum { WIFI_SCAN_REGION_ASIA, WIFI_SCAN_REGION_NORTH_AMERICA };

// SSID is 32 characters

typedef struct MCU_PACK {
  u8 channel;
  u8 slot_count;
  u8 slot_time_ms;
  u8 probe_count;
  s8 rssi_threshold;
  u8 scan_region;
  u8 is_passive;
} wifi_scan_attributes_t;

typedef struct MCU_PACK {
  char ssid[32];
  u8 bssid[6]; // mac address
  u8 idx;
  u8 channel;
  u8 security;
  s8 rssi;
  u8 resd[2];
} wifi_ssid_info_t;

typedef struct MCU_PACK {
  u8 password[64];
} wifi_auth_info_t;

typedef struct MCU_PACK {
  u32 ip_address;
  u32 gateway_address;
  u32 dns_address;
  u32 subnet_mask;
  u32 lease_time_s;
} wifi_ip_info_t;

#define WIFI_API_INFO_RESD (0x55)

typedef struct MCU_PACK {
  char ssid[32];
  u8 security;
  u8 rssi;
  u8 is_connected;
  u8 resd0;
  wifi_ip_info_t ip;
  u8 mac_address[NETIF_MAX_MAC_ADDRESS_SIZE];
} wifi_info_t;

// WPS is wifi protected setup

typedef struct {
  api_t sos_api;
  int (*init)(void **context);
  void (*deinit)(void **context);

  int (*connect)(
    void *context,
    const wifi_ssid_info_t *ssid_info,
    const wifi_auth_info_t *auth);
  int (*disconnect)(void *context);
  int (*start_scan)(void *context, const wifi_scan_attributes_t *attributes);
  int (*get_scan_count)(void *context);
  int (*get_ssid_info)(void *context, u8 idx, wifi_ssid_info_t *dest);
  int (*get_info)(void *context, wifi_info_t *info);
  int (*set_mode)(void *context);
  int (*set_mac_address)(void *context, u8 mac_address[6]);
  int (*get_mac_address)(void *context, u8 mac_address[6]);
  int (*get_factory_mac_address)(void *context, u8 mac_address[6]);
  int (*set_ip_address)(void *context, const wifi_ip_info_t *static_ip_address);

  int (*set_sleep_mode)(void *context);
  int (*sleep)(void *context, u32 sleep_time_ms);
  int (*set_device_name)(void *context, const char *name);
  int (*set_tx_power)(void *context, u8 power_level);

  // DHCP mode, STA, AP, P2P modes monitor modes
  // SNTP (time syncing)
  // set system time
  // get system time

} wifi_api_t;

extern const wifi_api_t wifi_api;

#if !defined __link
#define WIFI_API_REQUEST MCU_API_REQUEST_CODE('w', 'i', 'f', 'i')
#else
#define WIFI_API_REQUEST &wifi_api
#endif

#endif // SOS_API_WIFI_API_H
