
#ifndef USB_FLAGS_H_
#define USB_FLAGS_H_

#define __packed

typedef struct _EP_LIST {
  uint32_t  buf_ptr;
  uint32_t  buf_length;
} EP_LIST;

#if defined USE_DOUBLE_BUFFER
#define BUFFER_MULT 2
#else
#define BUFFER_MULT 2
#endif

#define USB_EPLIST_ADDR		0x20004000
#define USB_EPDATA_ADDR		(USB_EPLIST_ADDR + 0x100)
#define USB_EPDATA_PAGE		(USB_EPDATA_ADDR&0xFFC00000)	/* Bit 22~31 is the page */
#define USB_RAM_LIMIT		0x800		/* maximum USB RAM size is 2K */

#if 0
/* Be careful if ISO communication is used that it could allocate more than 64 bytes. */
#define USB_EP0DATA_OUT		(USB_EPDATA_ADDR + 0x00)
#define USB_SETUP_BUF		(USB_EPDATA_ADDR + 0x40)
#define USB_EP0DATA_IN		(USB_EPDATA_ADDR + 0x80)

#define USB_EP1DATA_OUT0	(USB_EPDATA_ADDR + 0x100)		/* INT/BULK EPs, max EP 64 bytes */
#define USB_EP1DATA_OUT1	(USB_EPDATA_ADDR + 0x140)
#define USB_EP1DATA_IN0		(USB_EPDATA_ADDR + 0x180)
#define USB_EP1DATA_IN1		(USB_EPDATA_ADDR + 0x1C0)

#define USB_EP2DATA_OUT0	(USB_EPDATA_ADDR + 0x200)
#define USB_EP2DATA_OUT1	(USB_EPDATA_ADDR + 0x240)
#define USB_EP2DATA_IN0		(USB_EPDATA_ADDR + 0x280)
#define USB_EP2DATA_IN1		(USB_EPDATA_ADDR + 0x2C0)

#define USB_EP3DATA_OUT0	(USB_EPDATA_ADDR + 0x300)
#define USB_EP3DATA_OUT1	(USB_EPDATA_ADDR + 0x340)
#define USB_EP3DATA_IN0		(USB_EPDATA_ADDR + 0x380)
#define USB_EP3DATA_IN1		(USB_EPDATA_ADDR + 0x3C0)

#define USB_EP4DATA_OUT0	(USB_EPDATA_ADDR + 0x400)
#define USB_EP4DATA_OUT1	(USB_EPDATA_ADDR + 0x440)
#define USB_EP4DATA_IN0		(USB_EPDATA_ADDR + 0x480)
#define USB_EP4DATA_IN1		(USB_EPDATA_ADDR + 0x4C0)
#endif

#define BUF_ACTIVE			(0x1U<<31)
#define EP_DISABLED			(0x1<<30)
#define EP_STALL			(0x1<<29)
#define EP_RESET			(0x1<<28)
#define EP_ISO_TYPE			(0x1<<26)

/* USB Device Command Status */
#define USB_EN              (0x1<<7)  /* Device Enable */
#define USB_SETUP_RCVD      (0x1<<8)  /* SETUP token received */
#define USB_PLL_ON          (0x1<<9)  /* PLL is always ON */
#define USB_LPM             (0x1<<11) /* LPM is supported */
#define USB_IntOnNAK_AO     (0x1<<12) /* Device Interrupt on NAK BULK OUT */
#define USB_IntOnNAK_AI     (0x1<<13) /* Device Interrupt on NAK BULK IN */
#define USB_IntOnNAK_CO     (0x1<<14) /* Device Interrupt on NAK CTRL OUT */
#define USB_IntOnNAK_CI     (0x1<<15) /* Device Interrupt on NAK CTRL IN */
#define USB_DCON            (0x1<<16) /* Device connect */
#define USB_DSUS            (0x1<<17) /* Device Suspend */
#define USB_LPM_SUS         (0x1<<19) /* LPM suspend */
#define USB_LPM_REWP        (0x1<<20) /* LPM Remote Wakeup */
#define USB_DCON_C          (0x1<<24) /* Device connection change */
#define USB_DSUS_C          (0x1<<25) /* Device SUSPEND change */
#define USB_DRESET_C        (0x1<<26) /* Device RESET */
#define USB_VBUS_DBOUNCE    (0x1<<28) /* Device VBUS detect */

/* Device Interrupt Bit Definitions */
#define EP0_INT             (0x1<<0)
#define EP1_INT             (0x1<<1)
#define EP2_INT             (0x1<<2)
#define EP3_INT             (0x1<<3)
#define EP4_INT             (0x1<<4)
#define EP5_INT             (0x1<<5)
#define EP6_INT             (0x1<<6)
#define EP7_INT             (0x1<<7)
#define EP8_INT             (0x1<<8)
#define EP9_INT             (0x1<<9)
#define FRAME_INT           (0x1<<30)
#define DEV_STAT_INT        (0x80000000)

/* Rx & Tx Packet Length Definitions */
#define PKT_LNGTH_MASK      0x000003FF

/* Error Status Register Definitions */
#define ERR_NOERROR			0x00
#define ERR_PID_ENCODE      0x01
#define ERR_UNKNOWN_PID     0x02
#define ERR_UNEXPECT_PKT    0x03
#define ERR_TCRC            0x04
#define ERR_DCRC            0x05
#define ERR_TIMEOUT         0x06
#define ERR_BABBIE          0x07
#define ERR_EOF_PKT         0x08
#define ERR_TX_RX_NAK		0x09
#define ERR_SENT_STALL      0x0A
#define ERR_BUF_OVERRUN     0x0B
#define ERR_SENT_EPT_PKT    0x0C
#define ERR_BIT_STUFF       0x0D
#define ERR_SYNC            0x0E
#define ERR_TOGGLE_BIT      0x0F



#endif // USB_FLAGS_H_

