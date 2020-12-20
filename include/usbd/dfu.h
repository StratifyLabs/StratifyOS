// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef USBD_DFU_H_
#define USBD_DFU_H_

#include <sdk/types.h>

//Part of DFU descriptors
#define USBD_DFU_INTERFACE_NUM_ENDPOINTS 0x00 //Use control pipe
#define USBD_DFU_INTERFACE_CLASS 0xFE
#define USBD_DFU_INTERFACE_SUBCLASS 0x01
#define USBD_DFU_INTERFACE_PROTOCOL 0x02

//Run time descriptor is part of run time descriptors
#define USBD_DFU_RUNTIME_INTERFACE_ALTERNATE_SETTING 0x00
#define USBD_DFU_RUNTIME_INTERFACE_NUM_ENDPOINTS 0x00 //Use control pipe
#define USBD_DFU_RUNTIME_INTERFACE_CLASS USBD_DFU_INTERFACE_CLASS
#define USBD_DFU_RUNTIME_INTERFACE_SUBCLASS USBD_DFU_INTERFACE_SUBCLASS
#define USBD_DFU_RUNTIME_INTERFACE_PROTOCOL 0x01


#define USBD_DFU_ATTRIBUTES_WILL_DETACH (1<<3)
#define USBD_DFU_ATTRIBUTES_MANIFESTATION_TOLERANT (1<<2)
#define USBD_DFU_ATTRIBUTES_CAN_UPLOAD (1<<1)
#define USBD_DFU_ATTRIBUTES_CAN_DOWNLOAD (1<<0)

#define USBD_DFU_FUNCTIONAL_DESCRIPTOR_TYPE 0x21

#define USBD_DFU_SUBCLASS 0x00
#define USBD_DFU_REQUEST_TYPE 0x21

//DFU requests
#define USBD_DFU_DETACH 0
#define USBD_DFU_DNLOAD 1
#define USBD_DFU_UPLOAD 2
#define USBD_DFU_GETSTATUS 3
#define USBD_DFU_CLRSTATUS 4
#define USBD_DFU_GETSTATE 5
#define USBD_DFU_ABORT 6

enum {
	appIDLE,
	appDETACH,
	dfuIDLE,
	dfuDNLOAD_SYNC,
	dfuDNBUSY,
	dfuDNLOAD_IDLE,
	dfuMANIFEST_SYNC,
	dfuMANIFEST,
	dfuMANIFEST_WAIT_RESET,
	dfuUPLOAD_IDLE,
	dfuERROR
};

enum {
	OK,
	errTARGET,
	errFILE,
	errWRITE,
	errERASE,
	errCHECK_ERASED,
	errPROG,
	errVERFIY,
	errADDRESS,
	errNOTDONE,
	errFIRMWARE,
	errVENDOR,
	errUSBR,
	errUNKNOWN,
	errSTALLEDPKT
};

typedef struct MCU_PACK {
	u16 bcdDevice;
	u16 idProduct;
	u16 idVendor;
	u16 bcdDFU;
	char ucDfuSignature[3];
	u8 bLength;
	uint32_t dwCRC;
} usb_dev_dfu_file_suffix_t;

typedef struct MCU_PACK {
	u8 bLength;
	u8 bDescriptorType;
	u8 bmAttributes;
	u16 wDetachTimeOut;
	u16 wTransferSize;
	u16 bcdDFUVersion;
} usb_dev_dfu_func_descriptor_t;

typedef struct MCU_PACK {
	u8 bStatus;
	u8 bwPollTimeout[3];
	u8 bState;
	u8 iString;
} usb_dev_dfu_status_t;




#endif /* USBD_DFU_H_ */
