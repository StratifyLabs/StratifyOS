// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#ifndef USBD_STANDARD_H_
#define USBD_STANDARD_H_

#include "usbd/control.h"

int usbd_standard_request_handle_setup(usbd_control_t * context);

u32 usbd_standard_request_set_interface(usbd_control_t * context);
u32 usbd_standard_request_get_interface(usbd_control_t * context);
void * usbd_control_add_ptr(usbd_control_t * context, void * ptr, u32 n);
u32 usbd_standard_request_set_clear_feature (usbd_control_t * context, u32 sc);
u32 usbd_standard_request_get_descriptor(usbd_control_t * context);
u32 usbd_standard_request_set_address (usbd_control_t * context);
u32 usbd_standard_request_set_config (usbd_control_t * context);
u32 usbd_standard_request_get_config (usbd_control_t * context);
u32 usbd_standard_request_get_status (usbd_control_t * context);


#endif /* USBD_STANDARD_H_ */
