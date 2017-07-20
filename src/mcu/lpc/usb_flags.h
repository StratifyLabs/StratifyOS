
#ifndef USB_FLAGS_H_
#define USB_FLAGS_H_


// Device Interrupt Bit Definitions

#define FRAME_INT (1<<0)
#define EP_FAST_INT (1<<1)
#define EP_SLOW_INT (1<<2)
#define DEV_STAT_INT (1<<3)
#define CCEMTY_INT (1<<4)
#define CDFULL_INT (1<<5)
#define RxENDPKT_INT (1<<6)
#define TxENDPKT_INT (1<<7)
#define EP_RLZED_INT (1<<8)
#define ERR_INT (1<<9)

// Rx & Tx Packet Length Definitions
#define PKT_LNGTH_MASK      0x000003FF
#define PKT_DV              0x00000400
#define PKT_RDY             0x00000800

// USB Control Definitions
#define CTRL_RD_EN          0x00000001
#define CTRL_WR_EN          0x00000002

// Command Codes
#define USB_SIE_CMD_SET_ADDR        0x00D00500
#define USB_SIE_CMD_CFG_DEV         0x00D80500
#define USB_SIE_CMD_SET_MODE        0x00F30500
#define USB_SIE_CMD_RD_FRAME        0x00F50500
#define USB_SIE_DAT_RD_FRAME        0x00F50200
#define USB_SIE_CMD_RD_TEST         0x00FD0500
#define USB_SIE_DAT_RD_TEST         0x00FD0200
#define USB_SIE_CMD_SET_DEV_STAT    0x00FE0500
#define USB_SIE_CMD_GET_DEV_STAT    0x00FE0500
#define USB_SIE_DAT_GET_DEV_STAT    0x00FE0200
#define USB_SIE_CMD_GET_ERR_CODE    0x00FF0500
#define USB_SIE_DAT_GET_ERR_CODE    0x00FF0200
#define USB_SIE_CMD_RD_ERR_STAT     0x00FB0500
#define USB_SIE_DAT_RD_ERR_STAT     0x00FB0200
#define USB_SIE_DAT_WR_BYTE(x)     (0x00000100 | ((x) << 16))
#define USB_SIE_CMD_SEL_EP(x)      (0x00000500 | ((x) << 16))
#define USB_SIE_DAT_SEL_EP(x)      (0x00000200 | ((x) << 16))
#define USB_SIE_CMD_SEL_EP_CLRI(x) (0x00400500 | ((x) << 16))
#define USB_SIE_DAT_SEL_EP_CLRI(x) (0x00400200 | ((x) << 16))
#define USB_SIE_CMD_SET_EP_STAT(x) (0x00400500 | ((x) << 16))
#define USB_SIE_CMD_CLR_BUF         0x00F20500
#define USB_SIE_DAT_CLR_BUF         0x00F20200
#define USB_SIE_CMD_VALID_BUF       0x00FA0500

// Device Address Register Definitions
#define DEV_ADDR_MASK       0x7F
#define DEV_EN              0x80

// Device Configure Register Definitions
#define CONF_DVICE          0x01

// Device Mode Register Definitions
#define AP_CLK              0x01
#define INAK_CI             0x02
#define INAK_CO             0x04
#define INAK_II             0x08
#define INAK_IO             0x10
#define INAK_BI             0x20
#define INAK_BO             0x40

// Device Status Register Definitions
#define DEV_CON             0x01
#define DEV_CON_CH          0x02
#define DEV_SUS             0x04
#define DEV_SUS_CH          0x08
#define DEV_RST             0x10

// Error Code Register Definitions
#define ERR_EC_MASK         0x0F
#define ERR_EA              0x10

// Error Status Register Definitions
#define ERR_PID             0x01
#define ERR_UEPKT           0x02
#define ERR_DCRC            0x04
#define ERR_TIMOUT          0x08
#define ERR_EOP             0x10
#define ERR_B_OVRN          0x20
#define ERR_BTSTF           0x40
#define ERR_TGL             0x80

// Endpoint Select Register Definitions
#define EP_SEL_F            0x01
#define EP_SEL_ST           0x02
#define EP_SEL_STP          0x04
#define EP_SEL_PO           0x08
#define EP_SEL_EPN          0x10
#define EP_SEL_B_1_FULL     0x20
#define EP_SEL_B_2_FULL     0x40

// Endpoint Status Register Definitions
#define EP_STAT_ST          0x01
#define EP_STAT_DA          0x20
#define EP_STAT_RF_MO       0x40
#define EP_STAT_CND_ST      0x80

// Clear Buffer Register Definitions
#define CLR_BUF_PO          0x01


// DMA Interrupt Bit Definitions
#define EOT_INT             0x01
#define NDD_REQ_INT         0x02
#define SYS_ERR_INT         0x04


typedef struct MCU_PACK {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bEndpointAddress;
  uint8_t  bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t  bInterval;
} mcu_usbd_endpoint_descriptor_t;



#endif // USB_FLAGS_H_

