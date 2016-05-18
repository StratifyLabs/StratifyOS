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

/*! \addtogroup DMA LPC17XX Direct Memory Access
 * @{
 *
 * \ingroup LPC17XXDEV
 *
 * \details The Direct memory access allows for direct memory transfers from memory to memory or memory to/from
 * peripherals.
 *
 *
 */

/*! \file */

#ifndef _MCU_ARCH_LPC_DMA_H_
#define _MCU_ARCH_LPC_DMA_H_



#define DMA_STATIC

#include "mcu/dma.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DMA_MAX_CHANNEL 8
#define DMA_MAX_TRANSFER_SIZE 4095


enum {
	DMA_CTRL_BURST_SIZE1,
	DMA_CTRL_BURST_SIZE4,
	DMA_CTRL_BURST_SIZE8,
	DMA_CTRL_BURST_SIZE16,
	DMA_CTRL_BURST_SIZE32,
	DMA_CTRL_BURST_SIZE64,
	DMA_CTRL_BURST_SIZE128,
	DMA_CTRL_BURST_SIZE256
};

#define DMA_CTRL_TRANSFER_SIZE(x) (x & 0x0FFF)
#define DMA_CTRL_SRC_BURST_SIZE(x) ( (x & 0x07) << 12)
#define DMA_CTRL_DEST_BURST_SIZE(x) ( (x & 0x07) << 15)


#define DMA_CTRL_SRC_WIDTH_MASK (3<<18)
#define DMA_CTRL_SRC_WIDTH8 (0 << 18)
#define DMA_CTRL_SRC_WIDTH16 (1 << 18)
#define DMA_CTRL_SRC_WIDTH32 (2 << 18)
#define DMA_CTRL_DEST_WIDTH8 (0 << 21)
#define DMA_CTRL_DEST_WIDTH16 (1 << 21)
#define DMA_CTRL_DEST_WIDTH32 (2 << 21)
#define DMA_CTRL_SRC_INC (1<<26)
#define DMA_CTRL_DEST_INC (1<<27)
#define DMA_CTRL_ENABLE_TERMINAL_COUNT_INT (1<<31)

#define DMA_CFG_ENABLE_CHAN (1<<0)
#define DMA_CFG_SRC_PERIPH(x) ( (x & 0x0F) << 1)
#define DMA_CFG_DEST_PERIPH(x) ( (x & 0x0F) << 6)
#define DMA_CFG_TRANSFER_TYPE(x) (x<<11)
#define DMA_CFG_ENABLE_IE (1<<14)
#define DMA_CFG_ENABLE_ITC (1<<15)
#define DMA_CFG_HALT (1<<18)

#define DMA_CFG_ENABLE 0
#define DMA_CFG_BIT_ENDIAN 1

//Read only
#define DMA_CFG_ACTIVE (1<<17)


#define DMA_HIGH_PRIORITY 7
#define DMA_LOW_PRIORITY 3

#ifdef __lpc17xx
#define DMA_REQ_SSP0_TX	0
#define DMA_REQ_SSP0_RX	1
#define DMA_REQ_SSP1_TX	2
#define DMA_REQ_SSP1_RX	3
#define DMA_REQ_ADC 4
#define DMA_REQ_I2S0 5
#define DMA_REQ_I2S1 6
#define DMA_REQ_DAC 7
#define DMA_REQ_UART0_TX 8
#define DMA_REQ_MAT00 8
#define DMA_REQ_UART0_RX 9
#define DMA_REQ_MAT01 9
#define DMA_REQ_UART1_TX 10
#define DMA_REQ_MAT10 10
#define DMA_REQ_UART1_RX 11
#define DMA_REQ_MAT11 11
#define DMA_REQ_UART2_TX 12
#define DMA_REQ_MAT20 12
#define DMA_REQ_UART2_RX 13
#define DMA_REQ_MAT21 13
#define DMA_REQ_UART3_TX 14
#define DMA_REQ_MAT30 14
#define DMA_REQ_UART3_RX 15
#define DMA_REQ_MAT31 15
#endif

#ifdef LPCXX7X_8X
#define DMA_REQ_DAC 9
#endif


#if defined __lpc43xx
#define DMA_REQ_SPIFI 0x0000
#define DMA_REQ_SCT_MATCH 0x0100
#define DMA_REQ_SGPIO14 0x0200
#define DMA_REQ_TIMER3 0x0300
#define DMA_REQ_DAC 0x000F
#endif

#ifdef __cplusplus
}
#endif


#endif /* _MCU_ARCH_LPC_DMA_H_ */

/*! @} */
