/* Copyright 2011-2016 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include "mcu/mcu.h"
#include "mcu/core.h"
#include "cortexm/mpu.h"


//  __I  uint32_t TYPE;                         /*!< Offset: 0x00  MPU Type Register                              */
//  __IO uint32_t CTRL;                         /*!< Offset: 0x04  MPU Control Register                           */
//  __IO uint32_t RNR;                          /*!< Offset: 0x08  MPU Region RNR Register                     */
//  __IO uint32_t RBAR;                         /*!< Offset: 0x0C  MPU Region Base Address Register               */
//  __IO uint32_t RASR;                         /*!< Offset: 0x10  MPU Region Attribute and Size Register         */
//  __IO uint32_t RBAR_A1;                      /*!< Offset: 0x14  MPU Alias 1 Region Base Address Register       */
//  __IO uint32_t RASR_A1;                      /*!< Offset: 0x18  MPU Alias 1 Region Attribute and Size Register */
//  __IO uint32_t RBAR_A2;                      /*!< Offset: 0x1C  MPU Alias 2 Region Base Address Register       */
//  __IO uint32_t RASR_A2;                      /*!< Offset: 0x20  MPU Alias 2 Region Attribute and Size Register */
//  __IO uint32_t RBAR_A3;                      /*!< Offset: 0x24  MPU Alias 3 Region Base Address Register       */
//  __IO uint32_t RASR_A3;                      /*!< Offset: 0x28  MPU Alias 3 Region Attribute and Size Register */


int mpu_enable(){
	MPU->CTRL = (1<<0)|(1<<2); //enable the MPU -- give priv software access to background map
	return 0;
}

int mpu_disable(){
	MPU->CTRL = 0; //enable the MPU -- give priv software access to background map
	return 0;
}

mpu_size_t mpu_calc_size(uint32_t size){
	uint32_t shift;
	shift = 5;
	while ( (uint32_t)(1<<shift) < (uint32_t)size ){
		shift++;
		if ( shift == (MPU_SIZE_4GB+1) ){
			//size is too big
			return 0;
		}
	}
	return (mpu_size_t)(shift-1);
}


int mpu_set_region_access(int region, mpu_access_t access_type){
	uint32_t rasr;
	uint8_t valid_regions;

	//check if the region is valid
	valid_regions = ((MPU->TYPE >> 8) & 0xFF);
	if ( region >= valid_regions ){
		return -1;
	}

	MPU->RNR = (region & 0xFF);  //Set the region register
	rasr = MPU->RASR; //Get the access settings
	rasr &= ~(0x7<<24); //clear the current access
	rasr |= (access_type << 24);
	MPU->RASR = rasr;
	return 0;
}

int mpu_disable_region(int region){
	uint32_t rasr;
	uint8_t valid_regions;
	//check if the region is valid
	valid_regions = ((MPU->TYPE >> 8) & 0xFF);
	if ( region >= valid_regions ){
		return -1;
	}

	MPU->RNR = (region & 0xFF);  //Set the region register
	rasr = MPU->RASR;
	rasr &= ~(0x01);
	MPU->RASR = rasr;
	return 0;

}

int mpu_getnextpowerof2(int size){
	int i;
	for(i=31; i > 0; i--){
		if ( size & (1<<i) ){
			if ( size == (1<<i) ){
				return (1<<i);
			} else {
				return (1<<(i+1));
			}
		}
	}
	return 1;
}


int mpu_calc_region(int region,
		void * addr,
		int size,
		mpu_access_t access,
		mpu_memory_t type,
		int executable,
		uint32_t * rbar,
		uint32_t * rasr){
	uint32_t up_size;
	uint32_t addr_tmp;
	uint32_t subregion_size;
	uint8_t subregion_mask;
	uint8_t subregions;
	mpu_size_t mpu_size;


	//Get the next power of two size
	up_size = mpu_getnextpowerof2(size);
	if ( up_size < 32 ){
		return -1;
	}

	//Make sure the address is aligned to the size
	addr_tmp = (uint32_t)addr;

	if ( addr_tmp & ((up_size)-1) ){
		return -1; //this should be an alignment error
	}

	//Make sure the size is aligned with a subregion
	subregion_size = up_size >> 3; //Divide by 8

	if ( subregion_size == 0 ){
		return -1;
	}

	if ( (up_size % subregion_size) != 0 ){
		return -1;
	}

	subregions = up_size / subregion_size;
	subregion_mask = ~((1<<subregions) - 1);

	//Check the size
	mpu_size = mpu_calc_size(up_size);
	if ( mpu_size < MPU_SIZE_32B ){
		return -1;
	}

	*rasr = (mpu_size << 1)|(1<<0)|(access<<24)|(subregion_mask<<8);

	if ( !executable ){
		*rasr |= (1<<28);
	}

	switch(type){
	case MPU_MEMORY_EXTERNAL_SRAM:
		*rasr |= ((1<<16)|(1<<17)|(1<<18));
		break;
	case MPU_MEMORY_SRAM:
		*rasr |= ((1<<18)|(1<<17));
		break;
	case MPU_MEMORY_FLASH:
		*rasr |= (1<<17);
		break;
	case MPU_MEMORY_PERIPHERALS:
		*rasr |= ((1<<16)|(1<<18));
		break;
	}

	*rbar = (uint32_t)addr | ((1<<4)|region);
	return 0;
}

int mpu_enable_region(int region,
		void * addr,
		int size,
		mpu_access_t access,
		mpu_memory_t type,
		int executable){
	int err;
	uint32_t rasr;
	uint32_t rbar;
	uint8_t valid_regions;


	//check if the region is valid
	valid_regions = ((MPU->TYPE >> 8) & 0xFF);
	if ( region >= valid_regions ){
		return -1;
	}

	err = mpu_calc_region(region, addr, size, access, type, executable, &rbar, &rasr);
	if ( err < 0 ){
		return err;
	}


	MPU->RNR = rbar & 0x07;
	MPU->RASR = 0;
	MPU->RBAR = rbar & ~(0x1F);
	MPU->RASR = rasr;

	return 0;
}
