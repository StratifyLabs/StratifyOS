#ifndef SOS_API_SOS_API_H
#define SOS_API_SOS_API_H

#include "mcu/types.h"

typedef struct {
	const char * name;
	u32 version;
	const char * git_hash;
} sos_api_t;

#endif // SOS_API_SOS_API_H
