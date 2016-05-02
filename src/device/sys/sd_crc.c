/*
 * crc7.c
 *
 *  Created on: Feb 26, 2015
 *      Author: tgil
 */


#include <sys/types.h>
#include <stdint.h>

#include "mcu/mcu.h"
#include "mcu/core.h"


uint16_t crc16(uint16_t seed, const uint8_t * buffer, size_t nbyte){
#if defined LPC_CRC
	size_t i;
	LPC_CRC->MODE = 0x00; //CRC-CCITT
	LPC_CRC->SEED = seed;
	for(i=0; i < nbyte; i++){
#ifdef __lpc82x
		LPC_CRC->WRDATA8 = buffer[i];
#else
		LPC_CRC->WR_DATA_BYTE.DATA = buffer[i];
#endif
	}
	return LPC_CRC->SUM;
#else
	return 0;
#endif
}


uint8_t crc7(uint8_t seed, const uint8_t * chr, size_t len) {
	int i,a;
	unsigned char crc,data;
	crc=seed;
	for(a=0; a<len; a++){
		data=chr[a];
		for (i=0;i<8;i++) {
			crc <<= 1;
			if ((data & 0x80)^(crc & 0x80)){
				crc ^=0x09;
			}
			data <<= 1;
		}
	}
	crc=(crc<<1)|1;
	return(crc);
}
