#ifndef SOS_API_WIFI_API_H
#define SOS_API_WIFI_API_H

#include "sos_api.h"

typedef struct {
	sos_api_t sos_api;
	int (*init)(void ** context);
	void (*deinit)(void ** context);

	int (*connect)(void * context);
	int (*disconnect)(void * context);
	int (*scan)(void * context);
	int (*get_info)(void * context);
	int (*set_mode)(void * context);

	//get/set mac address
	//get RSSI
	//DHCP mode, STA, AP, P2P modes
	//set device name


} wifi_api_t;


#if !defined __link
#define WIFI_API_REQUEST MCU_API_REQUEST_CODE('w','i','f','i')
#endif

#endif // SOS_API_WIFI_API_H
