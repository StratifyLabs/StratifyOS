/*
 * @brief Basic CMSIS include file for LPC5410x M4 core
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __CMSIS_5410X_H_
#define __CMSIS_5410X_H_

#include "core_cm4.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CMSIS_5410X_M4 CHIP: LPC5410X M4 core CMSIS include file
 * @ingroup CHIP_5410X_CMSIS_DRIVERS
 * @{
 */

#if defined(__ARMCC_VERSION)
// Kill warning "#pragma push with no matching #pragma pop"
  #pragma diag_suppress 2525
  #pragma push
  #pragma anon_unions
#elif defined(__CWCC__)
  #pragma push
  #pragma cpp_extensions on
#elif defined(__GNUC__)
/* anonymous unions are enabled by default */
#elif defined(__IAR_SYSTEMS_ICC__)
//  #pragma push // FIXME not usable for IAR
  #pragma language=extended
#else
  #error Not supported compiler type
#endif

/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */

#if !defined(CORE_M4)
#error "CORE_M4 is not defined"
#endif

/** @defgroup CMSIS_5410X_M4_IRQ CHIP_5410X: LPC5410X M4 core peripheral interrupt numbers
 * @{
 */

typedef enum {
	/******  Cortex-M4 Processor Exceptions Numbers ***************************************************/
	Reset_IRQn                    = -15,	/*!< 1  Reset Vector, invoked on Power up and warm reset */
	NonMaskableInt_IRQn           = -14,	/*!< 2  Non maskable Interrupt, cannot be stopped or preempted */
	HardFault_IRQn                = -13,	/*!< 3  Hard Fault, all classes of Fault */
	MemoryManagement_IRQn         = -12,	/*!< 4  Memory Management, MPU mismatch, including Access Violation and No Match */
	BusFault_IRQn                 = -11,	/*!< 5  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory related Fault */
	UsageFault_IRQn               = -10,	/*!< 6  Usage Fault, i.e. Undef Instruction, Illegal State Transition */
	SVCall_IRQn                   =  -5,	/*!< 11  System Service Call via SVC instruction */
	DebugMonitor_IRQn             =  -4,	/*!< 12  Debug Monitor                    */
	PendSV_IRQn                   =  -2,	/*!< 14  Pendable request for system service */
	SysTick_IRQn                  =  -1,	/*!< 15  System Tick Timer                */

	/******  LPC5410X Specific Interrupt Numbers ********************************************************/
	WDT_IRQn                      = 0,		/*!< WWDT                                             */
	BOD_IRQn                      = 1,		/*!< BOD                                              */
	Reserved2_IRQn                = 2,		/*!< Reserved Interrupt                               */
	DMA_IRQn                      = 3,		/*!< DMA                                              */
	GINT0_IRQn                    = 4,		/*!< GINT0                                            */
	PIN_INT0_IRQn                 = 5,		/*!< PININT0                                          */
	PIN_INT1_IRQn                 = 6,		/*!< PININT1                                          */
	PIN_INT2_IRQn                 = 7,		/*!< PININT2                                          */
	PIN_INT3_IRQn                 = 8,		/*!< PININT3                                          */
	UTICK_IRQn                    = 9,		/*!< Micro-tick Timer interrupt                       */
	MRT_IRQn                      = 10,		/*!< Multi-rate timer interrupt                       */
	CT32B0_IRQn                   = 11,		/*!< CTMR0                                            */
	CT32B1_IRQn                   = 12,		/*!< CTMR1                                            */
	CT32B2_IRQn                   = 13,		/*!< CTMR2                                            */
	CT32B3_IRQn                   = 14,		/*!< CTMR3                                            */
	CT32B4_IRQn                   = 15,		/*!< CTMR4                                            */
	SCT0_IRQn                     = 16,		/*!< SCT                                              */
	UART0_IRQn                    = 17,		/*!< UART0                                            */
	UART1_IRQn                    = 18,		/*!< UART1                                            */
	UART2_IRQn                    = 19,		/*!< UART2                                            */
	UART3_IRQn                    = 20,		/*!< UART3                                            */
	I2C0_IRQn                     = 21,		/*!< I2C0                                             */
	I2C1_IRQn                     = 22,		/*!< I2C1                                             */
	I2C2_IRQn                     = 23,		/*!< I2C2                                             */
	SPI0_IRQn                     = 24,		/*!< SPI0                                             */
	SPI1_IRQn                     = 25,		/*!< SPI1                                             */
	ADC_SEQA_IRQn                 = 26,		/*!< ADC0 sequence A completion                       */
	ADC_SEQB_IRQn                 = 27,		/*!< ADC0 sequence B completion                       */
	ADC_THCMP_IRQn                = 28,		/*!< ADC0 threshold compare and error                 */
	RTC_IRQn                      = 29,		/*!< RTC alarm and wake-up interrupts                 */
	Reserved30_IRQn               = 30,		/*!< Reserved Interrupt                               */
	MAILBOX_IRQn                  = 31,		/*!< Mailbox                                          */
	GINT1_IRQn                    = 32,		/*!< GINT1                                            */
	PIN_INT4_IRQn                 = 33,		/*!< External Interrupt 4                             */
	PIN_INT5_IRQn                 = 34,		/*!< External Interrupt 5                             */
	PIN_INT6_IRQn                 = 35,		/*!< External Interrupt 6                             */
	PIN_INT7_IRQn                 = 36,		/*!< External Interrupt 7                             */
	Reserved37_IRQn               = 37,		/*!< Reserved Interrupt                               */
	Reserved38_IRQn               = 38,		/*!< Reserved Interrupt                               */
	Reserved39_IRQn               = 39,		/*!< Reserved Interrupt                               */
	RIT_IRQn                      = 40,		/*!< Repetitive Interrupt Timer                       */
	Reserved41_IRQn               = 41,		/*!< Reserved Interrupt                               */
	Reserved42_IRQn               = 42,		/*!< Reserved Interrupt                               */
	Reserved43_IRQn               = 43,		/*!< Reserved Interrupt                               */
	Reserved44_IRQn               = 44,		/*!< Reserved Interrupt                               */
} LPC5410X_IRQn_Type;

/**
 * @brief  GPIO port register block structure
 */
typedef struct {				/*!< GPIO_PORT Structure */
	__IO uint8_t B[128][32];	/*!< Offset 0x0000: Byte pin registers ports 0 to n; pins PIOn_0 to PIOn_31 */
	__IO uint32_t W[32][32];	/*!< Offset 0x1000: Word pin registers port 0 to n */
	__IO uint32_t DIR[32];		/*!< Offset 0x2000: Direction registers port n */
	__IO uint32_t MASK[32];		/*!< Offset 0x2080: Mask register port n */
	__IO uint32_t PIN[32];		/*!< Offset 0x2100: Portpin register port n */
	__IO uint32_t MPIN[32];		/*!< Offset 0x2180: Masked port register port n */
	__IO uint32_t SET[32];		/*!< Offset 0x2200: Write: Set register for port n Read: output bits for port n */
	__O  uint32_t CLR[32];		/*!< Offset 0x2280: Clear port n */
	__O  uint32_t NOT[32];		/*!< Offset 0x2300: Toggle port n */
} LPC_GPIO_T;

/**
 * @brief GPIO grouped interrupt register block structure
 */
typedef struct {					/*!< GPIO_GROUP_INTn Structure */
	__IO uint32_t  CTRL;			/*!< GPIO grouped interrupt control register */
	__I  uint32_t  RESERVED0[7];
	__IO uint32_t  PORT_POL[8];		/*!< GPIO grouped interrupt port polarity register */
	__IO uint32_t  PORT_ENA[8];		/*!< GPIO grouped interrupt port m enable register */
	uint32_t       RESERVED1[4072];
} LPC_GPIOGROUPINT_T;

/**
 * @brief Repetitive Interrupt Timer register block structure
 */
typedef struct {				/*!< RITIMER Structure */
	__IO uint32_t  COMPVAL;		/*!< Compare register */
	__IO uint32_t  MASK;		/*!< Mask register */
	__IO uint32_t  CTRL;		/*!< Control register */
	__IO uint32_t  COUNTER;		/*!< 32-bit counter */
	__IO uint32_t  COMPVAL_H;	/*!< Compare register, upper 16-bits */
	__IO uint32_t  MASK_H;		/*!< Compare register, upper 16-bits */
	__I  uint32_t  reserved;
	__IO uint32_t  COUNTER_H;	/*!< Counter register, upper 16-bits */
} LPC_RITIMER_T;

/**
 * @brief LPC5410X Pin Interrupt and Pattern Match register block structure
 */
typedef struct {			/*!< PIN_INT Structure */
	__IO uint32_t ISEL;		/*!< Pin Interrupt Mode register */
	__IO uint32_t IENR;		/*!< Pin Interrupt Enable (Rising) register */
	__IO uint32_t SIENR;	/*!< Set Pin Interrupt Enable (Rising) register */
	__IO uint32_t CIENR;	/*!< Clear Pin Interrupt Enable (Rising) register */
	__IO uint32_t IENF;		/*!< Pin Interrupt Enable Falling Edge / Active Level register */
	__IO uint32_t SIENF;	/*!< Set Pin Interrupt Enable Falling Edge / Active Level register */
	__IO uint32_t CIENF;	/*!< Clear Pin Interrupt Enable Falling Edge / Active Level address */
	__IO uint32_t RISE;		/*!< Pin Interrupt Rising Edge register */
	__IO uint32_t FALL;		/*!< Pin Interrupt Falling Edge register */
	__IO uint32_t IST;		/*!< Pin Interrupt Status register */
	__IO uint32_t PMCTRL;	/*!< GPIO pattern match interrupt control register          */
	__IO uint32_t PMSRC;	/*!< GPIO pattern match interrupt bit-slice source register */
	__IO uint32_t PMCFG;	/*!< GPIO pattern match interrupt bit slice configuration register */
} LPC_PIN_INT_T;

/**
 * @brief MRT register block structure
 */
typedef struct {
	__IO uint32_t INTVAL;	/*!< Timer interval register */
	__O  uint32_t TIMER;	/*!< Timer register */
	__IO uint32_t CTRL;		/*!< Timer control register */
	__IO uint32_t STAT;		/*!< Timer status register */
} LPC_MRT_CH_T;

/**
 * @brief MRT register block structure
 */
typedef struct {
	LPC_MRT_CH_T CHANNEL[MRT_CHANNELS_NUM];
	uint32_t unused[44];
	__IO uint32_t MODCFG;
	__O  uint32_t IDLE_CH;
	__IO uint32_t IRQ_FLAG;
} LPC_MRT_T;

/** Individual mailbox IRQ structure */
typedef struct {
	__IO    uint32_t        IRQ;		/*!< Mailbox data */
	__O     uint32_t        IRQSET;		/*!< Mailbox data set bits only */
	__O     uint32_t        IRQCLR;		/*!< Mailbox dataclearset bits only */
	__I     uint32_t        RESERVED;
} LPC_MBOXIRQ_T;

/** Mailbox register structure */
typedef struct {						/*!< Mailbox register structure */
	LPC_MBOXIRQ_T           BOX[MAILBOX_AVAIL];	/*!< Mailbox, offset 0 = M0+, offset 1 = M4 */
	LPC_MBOXIRQ_T           RESERVED1[15 - MAILBOX_AVAIL];
	__I     uint32_t        RESERVED2[2];
	__IO    uint32_t        MUTEX;		/*!< Mutex */
} LPC_MBOX_T;

/**
 * @brief LPC5410X IO Configuration Unit register block structure
 */
typedef struct {			/*!< LPC5410X IOCON Structure */
	__IO uint32_t  PIO[2][32];
} LPC_IOCON_T;

/**
 * @brief CRC register block structure
 */
typedef struct {					/*!< CRC Structure */
	__IO    uint32_t    MODE;		/*!< CRC Mode Register */
	__IO    uint32_t    SEED;		/*!< CRC SEED Register */
	union {
		__I     uint32_t    SUM;	/*!< CRC Checksum Register. */
		__O     uint32_t    WRDATA32;	/*!< CRC Data Register: write size 32-bit*/
		__O     uint16_t    WRDATA16;	/*!< CRC Data Register: write size 16-bit*/
		__O     uint8_t     WRDATA8;	/*!< CRC Data Register: write size 8-bit*/
	};

} LPC_CRC_T;

/**
 * @brief ADC register block structure
 */
typedef struct {	/*!< ADCn Structure */
	__IO uint32_t CTRL;
	__O uint32_t RESERVED0;
	__IO uint32_t SEQ_CTRL[2];
	__IO uint32_t SEQ_GDAT[2];
	__IO uint32_t RESERVED1[2];
	__IO uint32_t DAT[12];
	__IO uint32_t THR_LOW[2];
	__IO uint32_t THR_HIGH[2];
	__IO uint32_t CHAN_THRSEL;
	__IO uint32_t INTEN;
	__IO uint32_t FLAGS;
	__IO uint32_t STARTUP;
	__IO uint32_t CALIBR;
} LPC_ADC_T;


/**
 * @brief DMA Controller shared registers structure
 */
typedef struct {					/*!< DMA shared registers structure */
	__IO uint32_t  ENABLESET;		/*!< DMA Channel Enable read and Set for all DMA channels */
	__I  uint32_t  RESERVED0;
	__O  uint32_t  ENABLECLR;		/*!< DMA Channel Enable Clear for all DMA channels */
	__I  uint32_t  RESERVED1;
	__I  uint32_t  ACTIVE;			/*!< DMA Channel Active status for all DMA channels */
	__I  uint32_t  RESERVED2;
	__I  uint32_t  BUSY;			/*!< DMA Channel Busy status for all DMA channels */
	__I  uint32_t  RESERVED3;
	__IO uint32_t  ERRINT;			/*!< DMA Error Interrupt status for all DMA channels */
	__I  uint32_t  RESERVED4;
	__IO uint32_t  INTENSET;		/*!< DMA Interrupt Enable read and Set for all DMA channels */
	__I  uint32_t  RESERVED5;
	__O  uint32_t  INTENCLR;		/*!< DMA Interrupt Enable Clear for all DMA channels */
	__I  uint32_t  RESERVED6;
	__IO  uint32_t  INTA;			/*!< DMA Interrupt A status for all DMA channels */
	__I  uint32_t  RESERVED7;
	__IO  uint32_t  INTB;			/*!< DMA Interrupt B status for all DMA channels */
	__I  uint32_t  RESERVED8;
	__O  uint32_t  SETVALID;		/*!< DMA Set ValidPending control bits for all DMA channels */
	__I  uint32_t  RESERVED9;
	__O  uint32_t  SETTRIG;			/*!< DMA Set Trigger control bits for all DMA channels */
	__I  uint32_t  RESERVED10;
	__O  uint32_t  ABORT;			/*!< DMA Channel Abort control for all DMA channels */
} LPC_DMA_COMMON_T;

/**
 * @brief DMA Controller shared registers structure
 */
typedef struct {					/*!< DMA channel register structure */
	__IO  uint32_t  CFG;				/*!< DMA Configuration register */
	__I   uint32_t  CTLSTAT;			/*!< DMA Control and status register */
	__IO  uint32_t  XFERCFG;			/*!< DMA Transfer configuration register */
	__I   uint32_t  RESERVED;
} LPC_DMA_CHANNEL_T;

/* On LPC540XX, Max DMA channel is 22 */
#define MAX_DMA_CHANNEL         (22)

/**
 * @brief DMA Controller register block structure
 */
typedef struct {					/*!< DMA Structure */
	__IO uint32_t  CTRL;			/*!< DMA control register */
	__I  uint32_t  INTSTAT;			/*!< DMA Interrupt status register */
	__IO uint32_t  SRAMBASE;		/*!< DMA SRAM address of the channel configuration table */
	__I  uint32_t  RESERVED2[5];
	LPC_DMA_COMMON_T DMACOMMON[1];	/*!< DMA shared channel (common) registers */
	__I  uint32_t  RESERVED0[225];
	LPC_DMA_CHANNEL_T DMACH[MAX_DMA_CHANNEL];	/*!< DMA channel registers */
} LPC_DMA_T;

/**
 * @brief High level ROM API structure
 */
typedef struct {
	const uint32_t reserved_usb;				/*!< Reserved */
	const uint32_t reserved_clib;				/*!< Reserved */
	const uint32_t reserved_can;				/*!< Reserved */
	const uint32_t reserved_pwrd;				/*!< Reserved */
	const uint32_t reserved_div;				/*!< Reserved */
	const uint32_t reserved_i2cd;				/*!< Reserved */
	const uint32_t reserved_dmad;				/*!< Reserved */
	const uint32_t reserved_spid;				/*!< Reserved */
	const uint32_t reserved_adcd;				/*!< Reserved */
	const uint32_t reserved_uartd;				/*!< Reserved */
	const uint32_t reserved_vfifo;				/*!< Reserved */
	const uint32_t reserved_usart;				/*!< Reserved */
} LPC_ROM_API_T;

#define CONFIG_SCT_nEV   (13)			/*!< Number of events */
#define CONFIG_SCT_nRG   (13)			/*!< Number of match/compare registers */
#define CONFIG_SCT_nOU   (8)			/*!< Number of outputs */
#define CONFIG_SCT_nIN   (8)			/*!< Number of outputs */

/**
 * @brief State Configurable Timer register block structure
 */
typedef struct {
	__IO  uint32_t CONFIG;				/*!< configuration Register (offset (0x000) */
	union {
		__IO uint32_t CTRL_U;			/*!< control Register */
		struct {
			__IO uint16_t CTRL_L;		/*!< low control register */
			__IO uint16_t CTRL_H;		/*!< high control register */
		};

	};

	union {
		__IO uint32_t LIMIT_U;			/*!< limit Register */
		struct {
			__IO uint16_t LIMIT_L;		/*!< limit register for counter L */
			__IO uint16_t LIMIT_H;		/*!< limit register for counter H */
		};

	};

	union {
		__IO uint32_t HALT_U;			/*!< halt Register */
		struct {
			__IO uint16_t HALT_L;		/*!< halt register for counter L */
			__IO uint16_t HALT_H;		/*!< halt register for counter H */
		};

	};

	union {
		__IO uint32_t STOP_U;			/*!< stop Register */
		struct {
			__IO uint16_t STOP_L;		/*!< stop register for counter L */
			__IO uint16_t STOP_H;		/*!< stop register for counter H */
		};

	};

	union {
		__IO uint32_t START_U;			/*!< start Register */
		struct {
			__IO uint16_t START_L;		/*!< start register for counter L */
			__IO uint16_t START_H;		/*!< start register for counter H */
		};

	};

	uint32_t RESERVED1[10];				/*!< 0x018 - 0x03C reserved */

	union {
		__IO uint32_t COUNT_U;			/*!< counter register (offset 0x040)*/
		struct {
			__IO uint16_t COUNT_L;		/*!< counter register for counter L */
			__IO uint16_t COUNT_H;		/*!< counter register for counter H */
		};

	};

	union {
		__IO uint32_t STATE_U;			/*!< State register */
		struct {
			__IO uint16_t STATE_L;		/*!< state register for counter L */
			__IO uint16_t STATE_H;		/*!< state register for counter H */
		};

	};

	__I  uint32_t INPUT;				/*!< input register */
	union {
		__IO uint32_t REGMODE_U;		/*!< RegMode register */
		struct {
			__IO uint16_t REGMODE_L;	/*!< match - capture registers mode register L */
			__IO uint16_t REGMODE_H;	/*!< match - capture registers mode register H */
		};

	};

	__IO uint32_t OUTPUT;				/*!< output register */
	__IO uint32_t OUTPUTDIRCTRL;		/*!< output counter direction Control Register */
	__IO uint32_t RES;					/*!< conflict resolution register */
	__IO uint32_t DMAREQ0;				/*!< DMA0 Request Register */
	__IO uint32_t DMAREQ1;				/*!< DMA1 Request Register */
	uint32_t RESERVED2[35];
	__IO uint32_t EVEN;		/*!< event enable register */
	__IO uint32_t EVFLAG;		/*!< event flag register */
	__IO uint32_t CONEN;	/*!< conflict enable register */
	__IO uint32_t CONFLAG;		/*!< conflict flag register */

	union {

		__IO union {	/*!< ... Match / Capture value */
			uint32_t U;		/*!<       SCTMATCH[i].U  Unified 32-bit register */

			struct {
				uint16_t L;		/*!<       SCTMATCH[i].L  Access to L value */
				uint16_t H;		/*!<       SCTMATCH[i].H  Access to H value */
			};

		} MATCH[CONFIG_SCT_nRG];

		__I union {
			uint32_t U;		/*!<       SCTCAP[i].U  Unified 32-bit register */

			struct {
				uint16_t L;		/*!<       SCTCAP[i].L  Access to L value */
				uint16_t H;		/*!<       SCTCAP[i].H  Access to H value */
			};

		} CAP[CONFIG_SCT_nRG];

	};

	uint32_t RESERVED3[48 + (16 - CONFIG_SCT_nRG)];

	union {

		__IO union {	/* 0x200-... Match Reload / Capture Control value */
			uint32_t U;		/*       SCTMATCHREL[i].U  Unified 32-bit register */

			struct {
				uint16_t L;		/*       SCTMATCHREL[i].L  Access to L value */
				uint16_t H;		/*       SCTMATCHREL[i].H  Access to H value */
			};

		} MATCHREL[CONFIG_SCT_nRG];

		__IO union {
			uint32_t U;		/*       SCTCAPCTRL[i].U  Unified 32-bit register */

			struct {
				uint16_t L;		/*       SCTCAPCTRL[i].L  Access to H value */
				uint16_t H;		/*       SCTCAPCTRL[i].H  Access to H value */
			};

		} CAPCTRL[CONFIG_SCT_nRG];

	};

	uint32_t RESERVED6[48 + (16 - CONFIG_SCT_nRG)];

	__IO struct {		/* 0x300-0x3FC  SCTEVENT[i].STATE / SCTEVENT[i].CTRL*/
		uint32_t STATE;		/* Event State Register */
		uint32_t CTRL;		/* Event Control Register */
	} EVENT[CONFIG_SCT_nEV];

	uint32_t RESERVED9[128 - 2 * CONFIG_SCT_nEV];		/*!< ...-0x4FC reserved */

	__IO struct {		/*!< 0x500-0x57C  SCTOUT[i].SET / SCTOUT[i].CLR */
		uint32_t SET;		/*!< Output n Set Register */
		uint32_t CLR;		/*!< Output n Clear Register */
	} OUT[CONFIG_SCT_nOU];

	uint32_t RESERVED10[191 - 2 * CONFIG_SCT_nOU];		/*!< ...-0x7F8 reserved */
	__I uint32_t MODULECONTENT;		/*!< 0x7FC Module Content */
} LPC_SCT_T;

/**
 * @brief SPI register block structure
 */
typedef struct {					/*!< SPI Structure */
	__IO uint32_t  CFG;				/*!< SPI Configuration register */
	__IO uint32_t  DLY;				/*!< SPI Delay register */
	__IO uint32_t  STAT;			/*!< SPI Status register */
	__IO uint32_t  INTENSET;		/*!< SPI Interrupt Enable Set register */
	__O uint32_t  INTENCLR;		/*!< SPI Interrupt Enable Clear register */
	__I  uint32_t  RXDAT;			/*!< SPI Receive Data register */
	__IO uint32_t  TXDATCTL;		/*!< SPI Transmit Data with Control register */
	__O  uint32_t  TXDAT;			/*!< SPI Transmit Data register */
	__IO uint32_t  TXCTRL;			/*!< SPI Transmit Control register */
	__IO uint32_t  DIV;				/*!< SPI clock Divider register */
	__I  uint32_t  INTSTAT;			/*!< SPI Interrupt Status register */
} LPC_SPI_T;

/**
 * @brief LPC5410X Main system configuration register block structure
 */
typedef struct {
	__IO uint32_t SYSMEMREMAP;			/*!< System Remap register */
	__I  uint32_t RESERVED0[4];
	__IO uint32_t SYSTCKCAL;			/*!< System Tick Calibration register */
	__I  uint32_t RESERVED1[1];
	__IO uint32_t NMISRC;				/*!< NMI Source select register */
	__IO uint32_t ASYNCAPBCTRL;			/*!< Asynch APB chiplet control register */
	__I  uint32_t RESERVED2[7];
	__IO uint32_t SYSRSTSTAT;			/*!< System Reset Stat register */
	__IO uint32_t PRESETCTRL[2];		/*!< Peripheral Reset Ctrl register */
	__IO uint32_t PRESETCTRLSET[2];		/*!< Peripheral Reset Ctrl Set register */
	__IO uint32_t PRESETCTRLCLR[2];		/*!< Peripheral Reset Ctrl Clr register */
	__IO uint32_t PIOPORCAP[2];			/*!< PIO Power-On Reset Capture register */
	__I  uint32_t RESERVED3[1];
	__IO uint32_t PIORESCAP[2];			/*!< PIO Pad Reset Capture register */
	__I  uint32_t RESERVED4[4];
	__IO uint32_t MAINCLKSELA;			/*!< Main Clk sel Source Sel A register */
	__IO uint32_t MAINCLKSELB;			/*!< Main Clk sel Source Sel B register */
	__I  uint32_t RESERVED5;
	__IO uint32_t ADCCLKSEL;			/*!< ADC Async Clk Sel register */
	__I  uint32_t RESERVED6;
	__IO uint32_t CLKOUTSELA;			/*!< Clk Out Sel Source A register */
	__IO uint32_t CLKOUTSELB;			/*!< Clk Out Sel Source B register */
	__I  uint32_t RESERVED7;
	__IO uint32_t SYSPLLCLKSEL;			/*!< System PLL Clk Selregister */
	__I  uint32_t RESERVED8[7];
	__IO uint32_t AHBCLKCTRL[2];		/*!< AHB Peripheral Clk Enable register */
	__IO uint32_t AHBCLKCTRLSET[2];		/*!< AHB Peripheral Clk Enable Set register */
	__IO uint32_t AHBCLKCTRLCLR[2];		/*!< AHB Peripheral Clk Enable Clr register */
	__I  uint32_t RESERVED9[2];
	__IO uint32_t SYSTICKCLKDIV;		/*!< Systick Clock divider register */
	__I  uint32_t RESERVED10[7];
	__IO uint32_t AHBCLKDIV;			/*!< Main Clk Divider register */
	__IO uint32_t RESERVED11;
	__IO uint32_t ADCCLKDIV;			/*!< ADC Async Clk Divider register */
	__IO uint32_t CLKOUTDIV;			/*!< Clk Out Divider register */
	__I  uint32_t RESERVED12[4];
	__IO uint32_t FREQMECTRL;			/*!< Frequency Measure Control register */
	__IO uint32_t FLASHCFG;				/*!< Flash Config register */
	__I  uint32_t RESERVED13[8];
	__IO uint32_t FIFOCTRL;				/*!< VFIFO control register */
	__I  uint32_t RESERVED14[14];
	__I  uint32_t RESERVED15[1];
	__I  uint32_t RESERVED16[2];
	__IO uint32_t RTCOSCCTRL;			/*!< RTC Oscillator Control register */
	__I  uint32_t RESERVED17[7];
	__IO uint32_t SYSPLLCTRL;			/*!< System PLL control register */
	__IO uint32_t SYSPLLSTAT;			/*!< PLL status register */
	__IO uint32_t SYSPLLNDEC;			/*!< PLL N decoder register */
	__IO uint32_t SYSPLLPDEC;			/*!< PLL P decoder register */
	__IO uint32_t SYSPLLSSCTRL[2];	/*!< Spread Spectrum control registers */
	__I  uint32_t RESERVED18[18];
	__IO uint32_t PDRUNCFG;				/*!< Power Down Run Config register */
	__IO uint32_t PDRUNCFGSET;			/*!< Power Down Run Config Set register */
	__IO uint32_t PDRUNCFGCLR;			/*!< Power Down Run Config Clr register */
	__I  uint32_t RESERVED19[9];
	__IO uint32_t STARTERP[2];			/*!< Start Signal Enable Register */
	__IO uint32_t STARTERSET[2];		/*!< Start Signal Enable Set Register */
	__IO uint32_t STARTERCLR[2];		/*!< Start Signal Enable Clr Register */
	__I  uint32_t RESERVED20[42];
	__I  uint32_t RESERVED20A[4];
	__I  uint32_t RESERVED21[57];
	__IO uint32_t JTAG_IDCODE;
	__IO uint32_t DEVICE_ID0;			/*!< Boot ROM and die revision register */
	__IO uint32_t DEVICE_ID1;			/*!< Boot ROM and die revision register */
} LPC_SYSCON_T;

/**
 * @brief LPC5410X Asynchronous system configuration register block structure
 */
typedef struct {
	__IO uint32_t AYSNCPRESETCTRL;		/*!< peripheral reset register */
	__IO uint32_t ASYNCPRESETCTRLSET;	/*!< peripheral reset Set register */
	__IO uint32_t ASYNCPRESETCTRLCLR;	/*!< peripheral reset Clr register */
	__I  uint32_t RESERVED0;
	__IO uint32_t ASYNCAPBCLKCTRL;		/*!< clk enable register */
	__IO uint32_t ASYNCAPBCLKCTRLSET;	/*!< clk enable Set register */
	__IO uint32_t ASYNCAPBCLKCTRLCLR;	/*!< clk enable Clr register */
	__I  uint32_t RESERVED1;
	__IO uint32_t ASYNCAPBCLKSELA;		/*!< clk source mux A register */
	__IO uint32_t ASYNCAPBCLKSELB;		/*!< clk source mux B register */
	__IO uint32_t ASYNCCLKDIV;			/*!< clk div register */
	__I  uint32_t RESERVED2;
	__IO uint32_t FRGCTRL;				/*!< Fraction Rate Generator Ctrl register */
} LPC_ASYNC_SYSCON_T;

/**
 * @brief 32-bit Standard timer register block structure
 */
typedef struct {					/*!< TIMERn Structure       */
	__IO uint32_t IR;				/*!< Interrupt Register. The IR can be written to clear interrupts. The IR can be read to identify which of eight possible interrupt sources are pending. */
	__IO uint32_t TCR;				/*!< Timer Control Register. The TCR is used to control the Timer Counter functions. The Timer Counter can be disabled or reset through the TCR. */
	__IO uint32_t TC;				/*!< Timer Counter. The 32 bit TC is incremented every PR+1 cycles of PCLK. The TC is controlled through the TCR. */
	__IO uint32_t PR;				/*!< Prescale Register. The Prescale Counter (below) is equal to this value, the next clock increments the TC and clears the PC. */
	__IO uint32_t PC;				/*!< Prescale Counter. The 32 bit PC is a counter which is incremented to the value stored in PR. When the value in PR is reached, the TC is incremented and the PC is cleared. The PC is observable and controllable through the bus interface. */
	__IO uint32_t MCR;				/*!< Match Control Register. The MCR is used to control if an interrupt is generated and if the TC is reset when a Match occurs. */
	__IO uint32_t MR[4];			/*!< Match Register. MR can be enabled through the MCR to reset the TC, stop both the TC and PC, and/or generate an interrupt every time MR matches the TC. */
	__IO uint32_t CCR;				/*!< Capture Control Register. The CCR controls which edges of the capture inputs are used to load the Capture Registers and whether or not an interrupt is generated when a capture takes place. */
	__IO uint32_t CR[4];			/*!< Capture Register. CR is loaded with the value of TC when there is an event on the CAPn.0 input. */
	__IO uint32_t EMR;				/*!< External Match Register. The EMR controls the external match pins MATn.0-3 (MAT0.0-3 and MAT1.0-3 respectively). */
	__I  uint32_t RESERVED0[12];
	__IO uint32_t CTCR;				/*!< Count Control Register. The CTCR selects between Timer and Counter mode, and in Counter mode selects the signal and edge(s) for counting. */
	__IO uint32_t PWMC;
} LPC_TIMER_T;

typedef struct {		/* UART registers Structure          */
	__IO uint32_t CFG;				/*!< Offset: 0x000 Configuration register  */
	__IO uint32_t CTL;				/*!< Offset: 0x004 Control register */
	__IO uint32_t STAT;				/*!< Offset: 0x008 Status register */
	__IO uint32_t INTENSET;			/*!< Offset: 0x00C Interrupt Enable Read and Set register */
	__O  uint32_t INTENCLR;			/*!< Offset: 0x010 Interrupt Enable Clear register */
	__I  uint32_t RXDAT;		/*!< Offset: 0x014 Receiver Data register */
	__I  uint32_t RXDATSTAT;	/*!< Offset: 0x018 Rx Data with status */
	__O  uint32_t TXDAT;			/*!< Offset: 0x01C Transmitter Data Register */
	__IO uint32_t BRG;				/*!< Offset: 0x020 Baud Rate Generator register */
	__I  uint32_t INTSTAT;	/*!< Offset: 0x024 Interrupt Status register */
	__IO uint32_t OSR;				/*!< Offset: 0x028 Oversampling register */
	__IO uint32_t ADR;				/*!< Offset: 0x02C Address register (for automatic address matching) */
} LPC_USART_T;

/**
 * @brief Micro Tick register block structure
 */
typedef struct {
	__IO uint32_t CTRL;				/*!< UTick Control register */
	__IO uint32_t STATUS;			/*!< UTick Status register */
} LPC_UTICK_T;

/**
 * @brief Windowed Watchdog register block structure
 */
typedef struct {				/*!< WWDT Structure         */
	__IO uint32_t  MOD;			/*!< Watchdog mode register. This register contains the basic mode and status of the Watchdog Timer. */
	__IO uint32_t  TC;			/*!< Watchdog timer constant register. This register determines the time-out value. */
	__O  uint32_t  FEED;		/*!< Watchdog feed sequence register. Writing 0xAA followed by 0x55 to this register reloads the Watchdog timer with the value contained in WDTC. */
	__I  uint32_t  TV;			/*!< Watchdog timer value register. This register reads out the current value of the Watchdog timer. */
	__I  uint32_t  RESERVED0;
	__IO uint32_t  WARNINT;		/*!< Watchdog warning interrupt register. This register contains the Watchdog warning interrupt compare value. */
	__IO uint32_t  WINDOW;		/*!< Watchdog timer window register. This register contains the Watchdog window value. */
} LPC_WWDT_T;

/* Main memory addresses */
#define LPC_FLASHMEM_BASE          0x00000000UL
#define LPC_SRAM0_BASE             0x02000000UL
#define LPC_SRAM1_BASE             0x02010000UL
#define LPC_ROM_BASE               0x03000000UL
#define LPC_SRAM2_BASE             0x03400000UL
#define LPC_GPIO_PORT_BASE         0x1C000000UL
#define LPC_DMA_BASE               0x1C004000UL
#define LPC_CRC_BASE               0x1C010000UL
#define LPC_SCT_BASE               0x1C018000UL
#define LPC_MBOX_BASE              0x1C02C000UL
#define LPC_ADC_BASE               0x1C034000UL
#define LPC_FIFO_BASE              0x1C038000UL

/* APB0 peripheral group addresses */
#define LPC_SYSCON_BASE            0x40000000UL
#define LPC_TIMER2_BASE            0x40004000UL
#define LPC_TIMER3_BASE            0x40008000UL
#define LPC_TIMER4_BASE            0x4000C000UL
#define LPC_GPIO_GROUPINT0_BASE    0x40010000UL
#define LPC_GPIO_GROUPINT1_BASE    0x40014000UL
#define LPC_PIN_INT_BASE           0x40018000UL
#define LPC_IOCON_BASE             0x4001C000UL
#define LPC_UTICK_BASE             0x40020000UL
#define LPC_FMC_BASE               0x40024000UL
#define LPC_PMU_BASE               0x4002C000UL
#define LPC_WWDT_BASE              0x40038000UL
#define LPC_RTC_BASE               0x4003C000UL

/* APB1 peripheral group addresses */
#define LPC_ASYNC_SYSCON_BASE      0x40080000UL
#define LPC_USART0_BASE            0x40084000UL
#define LPC_USART1_BASE            0x40088000UL
#define LPC_USART2_BASE            0x4008C000UL
#define LPC_USART3_BASE            0x40090000UL
#define LPC_I2C0_BASE              0x40094000UL
#define LPC_I2C1_BASE              0x40098000UL
#define LPC_I2C2_BASE              0x4009C000UL
#define LPC_SPI0_BASE              0x400A4000UL
#define LPC_SPI1_BASE              0x400A8000UL
#define LPC_TIMER0_BASE            0x400B4000UL
#define LPC_TIMER1_BASE            0x400B8000UL
#define LPC_INMUX_BASE             0x40050000UL
#define LPC_RITIMER_BASE           0x40070000UL
#define LPC_MRT_BASE               0x40074000UL

/* Main memory register access */
#define LPC_GPIO           ((LPC_GPIO_T            *) LPC_GPIO_PORT_BASE)
#define LPC_DMA            ((LPC_DMA_T             *) LPC_DMA_BASE)
#define LPC_CRC            ((LPC_CRC_T             *) LPC_CRC_BASE)
#define LPC_SCT            ((LPC_SCT_T             *) LPC_SCT_BASE)
#define LPC_MBOX           ((LPC_MBOX_T            *) LPC_MBOX_BASE)
#define LPC_ADC            ((LPC_ADC_T             *) LPC_ADC_BASE)
#define LPC_FIFO           ((LPC_FIFO_T            *) LPC_FIFO_BASE)

/* APB0 peripheral group register access */
#define LPC_SYSCON         ((LPC_SYSCON_T          *) LPC_SYSCON_BASE)
#define LPC_TIMER2         ((LPC_TIMER_T           *) LPC_TIMER2_BASE)
#define LPC_TIMER3         ((LPC_TIMER_T           *) LPC_TIMER3_BASE)
#define LPC_TIMER4         ((LPC_TIMER_T           *) LPC_TIMER4_BASE)
#define LPC_GINT           ((LPC_GPIOGROUPINT_T    *) LPC_GPIO_GROUPINT0_BASE)
#define LPC_PININT         ((LPC_PIN_INT_T         *) LPC_PIN_INT_BASE)
#define LPC_IOCON          ((LPC_IOCON_T           *) LPC_IOCON_BASE)
#define LPC_UTICK          ((LPC_UTICK_T           *) LPC_UTICK_BASE)
#define LPC_WWDT           ((LPC_WWDT_T            *) LPC_WWDT_BASE)
#define LPC_RTC            ((LPC_RTC_T             *) LPC_RTC_BASE)

/* APB1 peripheral group register access */
#define LPC_ASYNC_SYSCON   ((LPC_ASYNC_SYSCON_T    *) LPC_ASYNC_SYSCON_BASE)
#define LPC_USART0         ((LPC_USART_T           *) LPC_USART0_BASE)
#define LPC_USART1         ((LPC_USART_T           *) LPC_USART1_BASE)
#define LPC_USART2         ((LPC_USART_T           *) LPC_USART2_BASE)
#define LPC_USART3         ((LPC_USART_T           *) LPC_USART3_BASE)
#define LPC_I2C0           ((LPC_I2C_T             *) LPC_I2C0_BASE)
#define LPC_I2C1           ((LPC_I2C_T             *) LPC_I2C1_BASE)
#define LPC_I2C2           ((LPC_I2C_T             *) LPC_I2C2_BASE)
#define LPC_SCT0           LPC_SCT
#define LPC_SPI0           ((LPC_SPI_T             *) LPC_SPI0_BASE)
#define LPC_SPI1           ((LPC_SPI_T             *) LPC_SPI1_BASE)
#define LPC_TIMER0         ((LPC_TIMER_T           *) LPC_TIMER0_BASE)
#define LPC_TIMER1         ((LPC_TIMER_T           *) LPC_TIMER1_BASE)
#define LPC_INMUX          ((LPC_INMUX_T           *) LPC_INMUX_BASE)
#define LPC_RITIMER        ((LPC_RITIMER_T         *) LPC_RITIMER_BASE)
#define LPC_MRT            ((LPC_MRT_T             *) LPC_MRT_BASE)
#define LPC_PMU            ((LPC_PMU_T             *) LPC_PMU_BASE)


/**
 * @}
 */

/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/** @defgroup CMSIS_5410X_M4_COMMON CHIP: LPC5410X M4 core Cortex CMSIS definitions
 * @{
 */

/* Configuration of the Cortex-M4 Processor and Core Peripherals */
#define __CM4_REV                 0x0001	/*!< Cortex-M4 Core Revision                          */
#define __MPU_PRESENT             1			/*!< MPU present or not                               */
#define __NVIC_PRIO_BITS          3			/*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    0			/*!< Set to 1 if different SysTick Config is used     */
#define __FPU_PRESENT             1

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CMSIS_5410X_H_ */
