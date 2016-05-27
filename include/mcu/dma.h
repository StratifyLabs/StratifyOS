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

/*! \addtogroup DMA Direct Memory Access (DMA)
 * @{
 *
 * \ingroup CORE
 *
 * \details The Direct memory access allows for direct memory transfers from memory to memory or
 * memory to/from peripherals.  Various peripheral APIs use the DMA to asynchronously read/write
 * peripheral data.  The DMA should not be used directly by the application because it
 * may conflict with peripheral IO's use of the DMA.
 *
 *
 *
 */

/*! \file
 * \brief Direct Memory Access Header File
 *
 */

#ifndef DMA_H_
#define DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef dma_no_sleep_ram
/* \details This attribute can be applied to RAM so
 * that it is stored in RAM that is not shut off
 * when the processor sleeps.  This allows DMA operations
 * to continue during sleep.
 *
 * Example:
 * \code
 * dma_no_sleep_ram static char buffer[512];
 * \endcode
 */
#define dma_no_sleep_ram
#endif

typedef struct dma_lli_t{
	const void * src;
	void * dest;
	struct dma_lli_t * next;
	uint32_t ctrl;
} dma_lli_t;


/*! \details This lists the valid DMA operations.
 */
enum {
	DMA_MEM_TO_MEM /*! A memory to memory DMA operation */,
	DMA_MEM_TO_PERIPH /*! A memory to peripheral DMA operation */,
	DMA_PERIPH_TO_MEM /*! A peripheral to memory DMA operation */,
	DMA_PERIPH_TO_PERIPH /*! A peripheral to peripheral DMA operation */
};


/*! \details This function opens the DMA peripheral.
 */
void _mcu_dma_init(int mode /*! Flags which specify how the DMA is configured */) MCU_PRIV_CODE;



/*! \details This function halts the specified DMA channel.
 */
int _mcu_dma_halttransfer(int chan /*! The channel to halt */) MCU_PRIV_CODE;

/*! \details This function closes and disables the DMA peripheral.
 */
void _mcu_dma_exit() MCU_PRIV_CODE;

int _mcu_dma_transferlist(int operation,
		int chan,
		dma_lli_t * linked_list,
		mcu_callback_t callback,
		void * context,
		uint32_t dest_periph,
		uint32_t src_periph
		) MCU_PRIV_CODE;

int _mcu_dma_transfer(int operation,
		int chan,
		void * dest,
		const void * src,
		int ctrl,
		mcu_callback_t cb,
		void * context,
		uint32_t dest_periph,
		uint32_t src_periph) MCU_PRIV_CODE;


#ifdef __cplusplus
}
#endif

#endif /* DMA_H_ */

/*! @} */
