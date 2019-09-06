/* Copyright 2011-2018 Tyler Gilbert; 
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

/*! \addtogroup MPU Memory Protection Unit (MPU)
 * @{
 *
 * \ingroup CORE
 *
 * \details
 *
 */

/*! \file
 * \brief Watch Dog Timer Header File
 *
 */

#ifndef MPU_H_
#define MPU_H_

#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPU_MEMORY_NON_EXECUTABLE 0
#define MPU_MEMORY_EXECUTABLE 1

/*! \details This lists the types of memory
 * for the memory protection unit.
 */
typedef enum {
	MPU_MEMORY_FLASH /*! Flash Memory */,
	MPU_MEMORY_SRAM /*! SRAM */,
	MPU_MEMORY_EXTERNAL_SRAM /*! External SRAM */,
	MPU_MEMORY_PERIPHERALS /*! Peripheral Registers */,
	MPU_MEMORY_LCD /*! Peripheral Registers */
} mpu_memory_t;


/*! \details This lists the values for the size of memory
 * when using memory protection.
 *
 */
typedef enum {
	MPU_SIZE_RESERVED0 = -1, //register is one less that the exponent
	MPU_SIZE_RESERVED1,
	MPU_SIZE_RESERVED2,
	MPU_SIZE_RESERVED3,
	MPU_SIZE_RESERVED4,
	MPU_SIZE_32B /*! 32B */,
	MPU_SIZE_64B /*! 64B */,
	MPU_SIZE_128B /*! 128B */,
	MPU_SIZE_256B /*! 256B */,
	MPU_SIZE_512B /*! 512B */,
	MPU_SIZE_1KB /*! 1KB */,
	MPU_SIZE_2KB /*! 2KB */,
	MPU_SIZE_4KB /*! 4KB */,
	MPU_SIZE_8KB /*! 8KB */,
	MPU_SIZE_16KB /*! 16K2B */,
	MPU_SIZE_32KB /*! 32KB */,
	MPU_SIZE_64KB /*! 64KB */,
	MPU_SIZE_128KB /*! 128KB */,
	MPU_SIZE_256KB /*! 128KB */,
	MPU_SIZE_512KB /*! 512KB */,
	MPU_SIZE_1MB /*! 1MB */,
	MPU_SIZE_2MB /*! 2MB */,
	MPU_SIZE_4MB /*! 4MB */,
	MPU_SIZE_8MB /*! 8MB */,
	MPU_SIZE_16MB /*! 16MB */,
	MPU_SIZE_32MB /*! 32MB */,
	MPU_SIZE_64MB /*! 64MB */,
	MPU_SIZE_128MB /*! 128MB */,
	MPU_SIZE_256MB /*! 256MB */,
	MPU_SIZE_512MB /*! 512MB */,
	MPU_SIZE_1GB /*! 1GB */,
	MPU_SIZE_2GB /*! 2GB */,
	MPU_SIZE_4GB /*! 4GB */
} mpu_size_t;

typedef enum {
	MPU_ACCESS_NONE,
	MPU_ACCESS_PRW /*! Privileged read-write */,
	MPU_ACCESS_PRW_UR /*! Privileged read-write, unprivileged read */,
	MPU_ACCESS_PRW_URW /*! Privileged read-write, unprivileged read-write */,
	MPU_ACCESS_RESERVED,
	MPU_ACCESS_PR /*! Privileged read, unprivileged no access */,
	MPU_ACCESS_PR_UR /*! Privileged read, unprivileged read */
} mpu_access_t;


/*! \details This function enables memory protection.  It does not
 * configure any regions.
 */
int mpu_enable() MCU_ROOT_CODE;

/*! \details This function disables memory protection.  It does not change
 * the configuration settings for any regions.
 */
int mpu_disable() MCU_ROOT_CODE;


/*! \details This function calculates
 * the size value given the memory size in bytes.
 *
 * \return The MPU memory size value that is large enough to contain the specified memory
 */
mpu_size_t mpu_calc_size(uint32_t size /*! The size of the memory in bytes */) MCU_ROOT_CODE;

/*! \details This function enables the memory protection region.
 *
 */
int mpu_enable_region(int region /*! The region to enable */,
		const void * addr /*! The starting address */,
		u32 size /*! The size of the region */,
		mpu_access_t access /*! MPU Access value */,
		mpu_memory_t type /*! MPU memory type */,
		int executable /*! Non-zero to mark code as executable */) MCU_ROOT_CODE;

u32 mpu_calc_region(int region,
		const void * addr,
		u32 size,
		mpu_access_t access,
		mpu_memory_t type,
		int executable,
		uint32_t * rbar,
		uint32_t * rasr) MCU_ROOT_CODE;

int mpu_getnextpowerof2(int size) MCU_ROOT_CODE;

int mpu_dev_init() MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* MPU_H_ */

/*! @} */
