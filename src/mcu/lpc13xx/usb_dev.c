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

#include <fcntl.h>
#include "mcu/usb.h"
#include "mcu/gpio.h"
#include "usb_flags.h"

static uint32_t usb_irq_mask;
static usb_attr_t usb_ctl;
static void usb_connect(uint32_t con);
static inline void usb_cfg(int port, uint32_t cfg) MCU_ALWAYS_INLINE;
static inline void usb_set_addr(int port, uint32_t addr) MCU_ALWAYS_INLINE;
static inline void usb_reset_ep(int port, uint32_t endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_enable_ep(int port, uint32_t endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_disable_ep(int port, uint32_t endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_set_stall_ep(int port, uint32_t endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_clr_stall_ep(int port, uint32_t endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_cfg_ep(int port, void * ep_desc) MCU_ALWAYS_INLINE;
static inline void slow_ep_int(int episr) MCU_ALWAYS_INLINE;

#define USB_DEBUG

#ifdef USB_DEBUG
volatile unsigned int usbdev_stat;
#define USB_DEV_DEBUG(x) (usbdev_stat |= x)
#else
#define USB_DEV_DEBUG(x)
#endif

typedef struct {
	mcu_callback_t callback[DEV_USB_LOGICAL_ENDPOINT_COUNT];
	void * context[DEV_USB_LOGICAL_ENDPOINT_COUNT];
	volatile int write_pending;
	volatile int read_ready;
	void (*event_handler)(usb_spec_event_t,int);
	uint8_t connected;
} usb_local_t;

static usb_local_t usb_local;

static void clear_callbacks();
void clear_callbacks(){
	memset(usb_local.callback, 0, DEV_USB_LOGICAL_ENDPOINT_COUNT * sizeof(void(*)(int)));
}

#define EP_MSK_CTRL 0x0001 // Control Endpoint Logical Address Mask
#define EP_MSK_BULK 0xC924 // Bulk Endpoint Logical Address Mask
#define EP_MSK_INT  0x4492 // Interrupt Endpoint Logical Address Mask
#define EP_MSK_ISO  0x1248 // Isochronous Endpoint Logical Address Mask

static uint32_t calc_ep_addr (uint32_t endpoint_num);

static void usb_sie_wr_cmd(uint32_t cmd) MCU_NEVER_INLINE;
static uint32_t usb_sie_rd_dat() MCU_NEVER_INLINE;
static void usb_sie_wr_cmd_dat(uint32_t cmd, uint32_t val);
static void usb_sie_wr_cmd_ep(uint32_t ep_num, uint32_t cmd);
static uint32_t usb_sie_rd_cmd_dat(uint32_t cmd);


void usb_sie_wr_cmd(uint32_t cmd){
	LPC_USB->DevIntClr = CCEMTY_INT | CDFULL_INT;
	LPC_USB->CmdCode = cmd;
	while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
}

uint32_t usb_sie_rd_dat(){
	while ((LPC_USB->DevIntSt & CDFULL_INT) == 0);
	return LPC_USB->CmdData;
}

void usb_sie_wr_cmd_dat (uint32_t cmd, uint32_t val){
	usb_sie_wr_cmd(cmd);
	usb_sie_wr_cmd(val);
}

void usb_sie_wr_cmd_ep (uint32_t endpoint_num, uint32_t cmd){
	usb_sie_wr_cmd( USB_SIE_CMD_SEL_EP(calc_ep_addr(endpoint_num)));
	usb_sie_wr_cmd(cmd);
}

uint32_t usb_sie_rd_cmd_dat (uint32_t cmd){
	uint32_t dat;
	dat = (cmd & ~0xFFFF) | 0x0200;
	usb_sie_wr_cmd(cmd);
	usb_sie_wr_cmd(dat);
	return usb_sie_rd_dat();
}

int mcu_usb_seteventhandler(int port, void * ctl){
	usb_local.event_handler = (void(*)(usb_spec_event_t,int))ctl;
	return 0;
}

int _mcu_usb_dev_port_is_invalid(int port){
	if ( port != 0 ){
		return 1;
	}
	return 0;
}

void _mcu_usb_dev_power_on(int port){
	//Set callbacks to NULL
	clear_callbacks();
	usb_local.event_handler = NULL;
	_mcu_lpc_core_enable_pwr(PCUSB_REG);
}

void _mcu_usb_dev_power_off(int port){
	_mcu_cortexm_priv_disable_irq((void*)USB_IRQn);  //Enable the USB interrupt
	_mcu_lpc_core_disable_pwr(PCUSB_REG);
}

int _mcu_usb_dev_powered_on(int port){
	if ( _mcu_lpc_core_pwr_enabled(PCUSB_REG) ){
		return 1;
	}
	return 0;
}


int mcu_usb_getinfo(int port, void * ctl){
	memcpy(ctl, &usb_ctl, sizeof(usb_attr_t));
	return 0;
}

int mcu_usb_setattr(int port, void * ctl){
	usb_attr_t * ctlp;

	ctlp = (usb_attr_t *)ctl;

	if ( ctlp->pin_assign != 0 ){
		return -1;
	}

	if (ctlp->mode != USB_FLAG_SET_DEVICE){
		return -1;
	}

	_mcu_core_setusbclock(ctlp->crystal_freq);
	memcpy(&usb_ctl, ctl, sizeof(usb_attr_t));

	usb_local.read_ready = 0;
	usb_local.write_pending = 0;

	_mcu_core_set_pinsel_func(0, 3, CORE_PERIPH_USB, 0);
	usb_irq_mask = DEV_STAT_INT | (0xFF << 1);

	_mcu_cortexm_priv_enable_irq((void*)USB_IRQn);  //Enable the USB interrupt
	mcu_usb_reset(0, NULL);
	usb_set_addr(0,0);
	return 0;
}

int mcu_usb_setaction(int port, void * ctl){
	usb_action_t * action = (usb_action_t*)ctl;
	int log_ep;

	log_ep = action->channel & 0x7F;
	if ( (log_ep < DEV_USB_LOGICAL_ENDPOINT_COUNT)  ){
		usb_local.callback[ log_ep ] = action->handler.callback;
		usb_local.context[ log_ep ] = action->handler.context;
	}

	return 0;
}

int mcu_usb_attach(int port, void * ctl){
	usb_connect(1);
	return 0;
}

int mcu_usb_detach(int port, void * ctl){
	usb_connect(0);
	return 0;
}

int mcu_usb_configure(int port, void * ctl){
	usb_cfg(port, (int)ctl);
	return 0;
}

int mcu_usb_setaddr(int port, void * ctl){
	usb_set_addr(port, (uint32_t)ctl);
	return 0;
}

int mcu_usb_resetep(int port, void * ctl){
	usb_reset_ep(port, (uint32_t)ctl);
	return 0;
}

int mcu_usb_enableep(int port, void * ctl){
	usb_enable_ep(port, (uint32_t)ctl);
	return 0;
}

int mcu_usb_disableep(int port, void * ctl){
	usb_disable_ep(port, (uint32_t)ctl);
	return 0;
}

int mcu_usb_stallep(int port, void * ctl){
	int ep = (int)ctl;
	usb_set_stall_ep(port, ep);
	return 0;
}

int mcu_usb_unstallep(int port, void * ctl){
	int ep = (int)ctl;
	usb_clr_stall_ep(port, ep);
	return 0;
}

int mcu_usb_cfgep(int port, void * ctl){
	usb_cfg_ep(port, ctl);
	return 0;
}

int mcu_usb_isconnected(int port, void * ctl){
	return usb_local.connected;
}

int mcu_usb_set_event_handler(int port, void * ctl){
	usb_local.event_handler = (void(*)(usb_spec_event_t,int))ctl;
	return 0;
}

int _mcu_usb_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	int loc = rop->loc;

	if ( loc > (DEV_USB_LOGICAL_ENDPOINT_COUNT-1) ){
		return -1;
	}

	//Synchronous read (only if data is ready) otherwise 0 is returned
	if ( usb_local.read_ready & (1<<loc) ){
		usb_local.read_ready &= ~(1<<loc);  //clear the read ready bit
		return mcu_usb_rd_ep(0, loc, rop->buf);
	} else {
		rop->nbyte = 0;
		if ( !(rop->flags & O_NONBLOCK) ){
			//If this is a blocking call, set the callback and context
			usb_local.callback[loc] = rop->handler.callback;
			usb_local.context[loc] = rop->handler.context;
		}
		return 0;
	}
}

int _mcu_usb_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	//Asynchronous write
	int ep;
	int port;
	int loc = wop->loc;

	port = DEVFS_GET_PORT(cfg);

	ep = (loc & 0x7F);

	if ( ep > (DEV_USB_LOGICAL_ENDPOINT_COUNT-1) ){
		return -1;
	}

	if ( usb_local.write_pending & (1<<ep) ){
		//return wait for op complete
		return -1;
	}

	usb_local.callback[ep] = wop->handler.callback;
	usb_local.context[ep] = wop->handler.context;

	usb_local.write_pending |= (1<<ep);
	wop->nbyte = mcu_usb_wr_ep(0, loc, wop->buf, wop->nbyte);

	if ( wop->nbyte < 0 ){
		usb_disable_ep( port, loc );
		usb_reset_ep( port, loc );
		usb_enable_ep( port, loc );
		usb_local.write_pending &= ~(1<<ep);
		return -2;
	}

	return 0;
}

uint32_t calc_ep_addr (uint32_t endpoint_num){
	uint32_t val;
	val = (endpoint_num & 0x0F) << 1;
	if (endpoint_num & 0x80){
		val += 1;
	}
	return (val);
}

/*! \details
 */
void usb_connect(uint32_t con){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_DEV_STAT, USB_SIE_DAT_WR_BYTE(con ? DEV_CON : 0));
}

int usb_connected(int port){
	return 1;
}

/*! \details
 */
int mcu_usb_reset(int port, void * cfg){
	LPC_USB->DevIntClr = 0x000FFFFF;
	LPC_USB->DevIntEn = usb_irq_mask;
	return 0;
}


/*! \details
 */
void usb_wakeup(int port){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_DEV_STAT, USB_SIE_DAT_WR_BYTE(DEV_CON));
}

/*! \details
 */
void usb_set_addr(int port, uint32_t addr){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_ADDR, USB_SIE_DAT_WR_BYTE(DEV_EN | addr)); /* Don't wait for next */
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_ADDR, USB_SIE_DAT_WR_BYTE(DEV_EN | addr)); /*  Setup Status Phase */
}


/*! \details
 */
void usb_cfg(int port, uint32_t cfg){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_CFG_DEV, USB_SIE_DAT_WR_BYTE(cfg ? CONF_DVICE : 0));
}

/*! \details
 */
void usb_cfg_ep(int port, void * ep_desc){
	//uint32_t num;
	//mcu_usb_ep_desc_t * ep_ptr;
	//ep_ptr = (mcu_usb_ep_desc_t*)ep_desc;
	//num = calc_ep_addr(ep_ptr->bEndpointAddress);
	//LPC_USB->USBReEp |= (1 << num);
	//LPC_USB->EpInd = num;
	//LPC_USB->USBMaxPSize = ep_ptr->wMaxPacketSize;


	//while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);
	//LPC_USB->DevIntClr = EP_RLZED_INT;
}



/*! \details
 */
void usb_enable_ep(int port, uint32_t endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
}


/*! \details
 */
void usb_disable_ep(int port, uint32_t endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(EP_STAT_DA));
}


/*! \details
 */
void usb_reset_ep(int port, uint32_t endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
}

/*! \details
 */
void usb_set_stall_ep(int port, uint32_t endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(EP_STAT_ST));
}


/*! \details
 */
void usb_clr_stall_ep(int port, uint32_t endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
}


/*! \details
 */
void usb_clr_ep_buf(int port, uint32_t endpoint_num){
	usb_sie_wr_cmd_ep(endpoint_num, USB_SIE_CMD_CLR_BUF);
}


int mcu_usb_rd_ep(int port, uint32_t endpoint_num, void * dest){
	uint32_t n;
	uint32_t * ptr;
	uint32_t size;
	uint32_t mask;
	int i;
	LPC_USB->Ctrl = ((endpoint_num & 0x0F) << 2) | CTRL_RD_EN;

	//There needs to be a delay between the time Ctrl is set and reading USBRxPLen due to the difference in clocks
	if ( _mcu_core_getclock() > 48000000 ){
		for(i=0; i < 8; i++){
			asm volatile("nop");
		}
	}

	ptr = (uint32_t*)dest;
	n = 0;
	mask = ((EP_MSK_ISO >> endpoint_num) & 1);
	size = LPC_USB->RxPLen;

	while(LPC_USB->Ctrl & CTRL_RD_EN){
		*ptr++ = LPC_USB->RxData;
		n += 4;
	}

	if (mask == 0){   // Non-Isochronous Endpoint
		//Clear the buffer so more data can be received
		usb_sie_wr_cmd_ep(endpoint_num, USB_SIE_CMD_CLR_BUF);
	}

	return size & 0x3FF;
}


/*! \details
 */
int mcu_usb_wr_ep(int port, uint32_t endpoint_num, const void * src, uint32_t size){
	uint32_t n;
	uint32_t * ptr = (uint32_t*)src;

	LPC_USB->Ctrl = ((endpoint_num & 0x0F) << 2) | CTRL_WR_EN;
	LPC_USB->TxPLen = size;

	n = 0;
	while( LPC_USB->Ctrl & CTRL_WR_EN ){
		if ( n < size ){
			LPC_USB->TxData = *ptr++;
		} else {
			LPC_USB->TxData = 0;
		}
		n+=4;
	}

	usb_sie_wr_cmd_ep(endpoint_num, USB_SIE_CMD_VALID_BUF);
	return size;
}



/*! \details This function services the USB interrupt request.
 */
void _mcu_core_usb_isr(){
	uint32_t device_interrupt_status;
	uint32_t tmp;
	int i;

	device_interrupt_status = LPC_USB->DevIntSt;     //Device interrupt status

	USB_DEV_DEBUG(0x01);

	if (device_interrupt_status & FRAME_INT){ //start of frame
		if ( usb_local.event_handler ){
			usb_local.event_handler(USB_SPEC_EVENT_SOF, 0);
		}
		LPC_USB->DevIntClr = FRAME_INT;
	}

	if (device_interrupt_status & DEV_STAT_INT){ //Status interrupt (Reset, suspend/resume or connect)



		for(tmp = 0; tmp < 1000000; tmp++){
			asm volatile("nop\n");
		}


		//usb_sie_wr_cmd(USB_SIE_CMD_GET_DEV_STAT);
		tmp = usb_sie_rd_cmd_dat(USB_SIE_CMD_GET_DEV_STAT);
		USB_DEV_DEBUG(0x02);



		if (tmp & DEV_RST){
			//mcu_usb_reset(0, NULL);
			usb_local.connected = 1;
			usb_local.write_pending = 0;
			usb_local.read_ready = 0;
			//if ( usb_local.event_handler ){ usb_local.event_handler(USB_SPEC_EVENT_RESET, 0); }
			USB_DEV_DEBUG(0x04);
		}

		if ( tmp == 0x0D ){
			usb_local.connected = 0;
			for(i = 1; i < DEV_USB_LOGICAL_ENDPOINT_COUNT; i++){
				if( usb_local.callback[i] != NULL ){
					if( usb_local.callback[i](usb_local.context[i], (const void*)-1) == 0 ){
						usb_local.callback[i] = NULL;
					}
				}
			}
			USB_DEV_DEBUG(0x08);
		}

		if (tmp & DEV_CON_CH){
			if ( usb_local.event_handler ){
				//usb_local.event_handler(USB_SPEC_EVENT_POWER, tmp);
			}
			USB_DEV_DEBUG(0x10);
		}

		if (tmp & DEV_SUS_CH){
			if (tmp & DEV_SUS){
				if ( usb_local.event_handler ){
					//usb_local.event_handler(USB_SPEC_EVENT_SUSPEND, tmp);
				}
				USB_DEV_DEBUG(0x20);
			} else {
				if ( usb_local.event_handler ){
					//usb_local.event_handler(USB_SPEC_EVENT_RESUME, tmp);
				}
				USB_DEV_DEBUG(0x40);
			}
		}


		LPC_USB->DevIntClr = DEV_STAT_INT|CCEMTY_INT|CDFULL_INT|RxENDPKT_INT|TxENDPKT_INT;
		return;
	}

	if (device_interrupt_status & EP_INT_MASK){ //Endpoint interrupts
		slow_ep_int(device_interrupt_status);
	}

	LPC_USB->DevIntClr = CCEMTY_INT|CDFULL_INT|RxENDPKT_INT|TxENDPKT_INT|EP_INT_MASK;
	return;
}

void slow_ep_int(int episr){
	uint32_t phy_ep, log_ep;
	uint32_t tmp;
	for (phy_ep = 0; phy_ep < USB_EP_NUM; phy_ep++){

		if ( (episr>>1) & (1 << phy_ep)){

			//Calculate the logical endpoint value (associated with the USB Spec)
			log_ep = phy_ep >> 1;


			//Clear the interrupt and select the endpoint -- this works differently than the lpc17xx
			tmp = usb_sie_rd_cmd_dat(USB_SIE_CMD_SEL_EP_CLRI(phy_ep));

			//Check for endpoint 0
			if ((phy_ep & 1) == 0){ //These are the OUT endpoints


				//Check for a setup packet
				if ( (phy_ep == 0) && (tmp & EP_SEL_STP) ){
					LPC_GPIO1->DATA |= (1<<2);
					USB_DEV_DEBUG(0x80);
					if (usb_local.callback[0]){
						usb_local.callback[0](usb_local.context, (const void*)USB_SETUP_EVENT);
					}
				} else {
					usb_local.read_ready |= (1<<log_ep);
					USB_DEV_DEBUG(0x100);
					if( usb_local.callback[log_ep] != NULL ){
						usb_local.callback[log_ep](usb_local.context[log_ep], (const void*)USB_OUT_EVENT);
						if ( log_ep > 0 ){
							usb_local.callback[log_ep] = NULL;
						}
					}
				}
			} else {  //These are the IN endpoints
				usb_local.write_pending &= ~(1<<log_ep);
				USB_DEV_DEBUG(0x200);
				if( usb_local.callback[log_ep] != NULL ){
					usb_local.callback[log_ep](usb_local.context[log_ep], (const void*)USB_IN_EVENT);
					if ( log_ep > 0 ){
						usb_local.callback[log_ep] = NULL;
					}
				}
			}

		}

	}
	LPC_USB->DevIntClr = EP_INT_MASK;
}









