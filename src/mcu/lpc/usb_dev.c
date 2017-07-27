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

#include <errno.h>
#include <fcntl.h>
#include "cortexm/cortexm.h"
#include "mcu/usb.h"
#include "mcu/pio.h"
#include "usb_flags.h"
#include "cortexm/cortexm.h"
#include "mcu/core.h"
#include "mcu/debug.h"
#include "mcu/boot_debug.h"


#if MCU_USB_PORTS > 0

static u32 usb_irq_mask;
static usb_attr_t usb_ctl;
static void usb_connect(u32 con);
static inline void usb_cfg(const devfs_handle_t * handle, u32 cfg) MCU_ALWAYS_INLINE;
static inline void usb_set_addr(const devfs_handle_t * handle, u32 addr) MCU_ALWAYS_INLINE;
static inline void usb_reset_ep(const devfs_handle_t * handle, u32 endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_enable_ep(const devfs_handle_t * handle, u32 endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_disable_ep(const devfs_handle_t * handle, u32 endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_set_stall_ep(const devfs_handle_t * handle, u32 endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_clr_stall_ep(const devfs_handle_t * handle, u32 endpoint_num) MCU_ALWAYS_INLINE;
static inline void usb_cfg_ep(const devfs_handle_t * handle, void * ep_desc) MCU_ALWAYS_INLINE;
static inline void slow_ep_int() MCU_ALWAYS_INLINE;

typedef struct {
	mcu_event_handler_t write[DEV_USB_LOGICAL_ENDPOINT_COUNT];
	mcu_event_handler_t read[DEV_USB_LOGICAL_ENDPOINT_COUNT];
	volatile u32 write_pending;
	volatile u32 read_ready;
	mcu_event_handler_t special_event_handler;
	u8 ref_count;
	u8 connected;
} usb_local_t;

static usb_local_t usb_local MCU_SYS_MEM;

static void clear_callbacks();
void clear_callbacks(){
	memset(usb_local.write, 0, DEV_USB_LOGICAL_ENDPOINT_COUNT * sizeof(mcu_event_handler_t));
	memset(usb_local.read, 0, DEV_USB_LOGICAL_ENDPOINT_COUNT * sizeof(mcu_event_handler_t));
}

#define EP_MSK_CTRL 0x0001 // Control Endpoint Logical Address Mask
#define EP_MSK_BULK 0xC924 // Bulk Endpoint Logical Address Mask
#define EP_MSK_INT  0x4492 // Interrupt Endpoint Logical Address Mask
#define EP_MSK_ISO  0x1248 // Isochronous Endpoint Logical Address Mask

static u32 calc_ep_addr (u32 endpoint_num);

//The following should be static
static void usb_sie_wr_cmd(u32 cmd) MCU_NEVER_INLINE;
static u32 usb_sie_rd_dat() MCU_NEVER_INLINE;
static void usb_sie_wr_cmd_dat(u32 cmd, u32 val);
static void usb_sie_wr_cmd_ep(u32 ep_num, u32 cmd);
static u32 usb_sie_rd_cmd_dat(u32 cmd);

#ifdef LPCXX7X_8X
static void configure_pin(const mcu_pin_t * pin, void * arg){
	LPC_USB->StCtrl &= ~0x03;
	if( pin->pin == 31 ){
		LPC_USB->StCtrl |= 0x03;
	}
	LPC_USB->USBClkCtrl &= ~(1<<3); //disable portsel clock
}
#else
#define configure_pin 0
#endif


void usb_sie_wr_cmd(u32 cmd){
	LPC_USB->DevIntClr = CCEMTY_INT | CDFULL_INT;
	LPC_USB->CmdCode = cmd;
	while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
}

u32 usb_sie_rd_dat(){
	while ((LPC_USB->DevIntSt & CDFULL_INT) == 0);
	return LPC_USB->CmdData;
}

void usb_sie_wr_cmd_dat (u32 cmd, u32 val){
	usb_sie_wr_cmd(cmd);
	usb_sie_wr_cmd(val);
}

void usb_sie_wr_cmd_ep (u32 endpoint_num, u32 cmd){
	usb_sie_wr_cmd( USB_SIE_CMD_SEL_EP(calc_ep_addr(endpoint_num)));
	usb_sie_wr_cmd(cmd);
}

u32 usb_sie_rd_cmd_dat (u32 cmd){
	u32 dat;
	dat = (cmd & ~0xFFFF) | 0x0200;
	usb_sie_wr_cmd(cmd);
	usb_sie_wr_cmd(dat);
	return usb_sie_rd_dat();
}

void mcu_usb_dev_power_on(const devfs_handle_t * handle){
	if ( usb_local.ref_count == 0 ){
		//Set callbacks to NULL
		usb_local.connected = 0;
		clear_callbacks();
		usb_local.special_event_handler.callback = 0;
		mcu_lpc_core_enable_pwr(PCUSB);
		LPC_USB->USBClkCtrl = 0x12; //turn on dev clk en and AHB clk en
		while( LPC_USB->USBClkCtrl != 0x12 ){}  //wait for clocks
	}
	usb_local.ref_count++;
}

void mcu_usb_dev_power_off(const devfs_handle_t * handle){
	if ( usb_local.ref_count > 0 ){
		if ( usb_local.ref_count == 1 ){
			cortexm_disable_irq((void*)(USB_IRQn));  //Enable the USB interrupt
			LPC_USB->USBClkCtrl = 0x0; //turn off dev clk en and AHB clk en
			while( LPC_USB->USBClkCtrl != 0 ){}
			mcu_lpc_core_disable_pwr(PCUSB);
		}
		usb_local.ref_count--;
	}
}

int mcu_usb_dev_is_powered(const devfs_handle_t * handle){
	if( mcu_lpc_core_pwr_enabled(PCUSB) ){
		return 1;
	}
	return 0;
}


int mcu_usb_getinfo(const devfs_handle_t * handle, void * ctl){
	memcpy(ctl, &usb_ctl, sizeof(usb_attr_t));
	return 0;
}

int mcu_usb_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;

	const usb_attr_t * attr = mcu_select_attr(handle, ctl);
	if( attr == 0 ){
		return -1;
	}

	if ((attr->o_flags & USB_FLAG_SET_DEVICE) == 0){
		errno = EINVAL;
		return -1 - offsetof(usb_attr_t, o_flags);
	}

	//Start the USB clock
	mcu_core_setusbclock(attr->freq);

	memcpy(&usb_ctl, ctl, sizeof(usb_attr_t));

	usb_local.read_ready = 0;
	usb_local.write_pending = 0;

	if( mcu_set_pin_assignment(
			&(attr->pin_assignment),
			MCU_CONFIG_PIN_ASSIGNMENT(usb_config_t, handle),
			MCU_PIN_ASSIGNMENT_COUNT(usb_pin_assignment_t),
			CORE_PERIPH_USB, port, configure_pin, 0) < 0 ){
		return -1;
	}

	usb_irq_mask = DEV_STAT_INT | EP_FAST_INT | EP_SLOW_INT;

	cortexm_enable_irq((void*)USB_IRQn);  //Enable the USB interrupt
	mcu_usb_reset(0, NULL);
	usb_set_addr(0,0);
	return 0;
}

void usb_connect(u32 con){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_DEV_STAT, USB_SIE_DAT_WR_BYTE(con ? DEV_CON : 0));
}

int mcu_usb_attach(const devfs_handle_t * handle, void * ctl){
	usb_connect(1);
	return 0;
}

int mcu_usb_detach(const devfs_handle_t * handle, void * ctl){
	usb_connect(0);
	return 0;
}

int mcu_usb_setaction(const devfs_handle_t * handle, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	int log_ep;
	int ret = -1;

	cortexm_set_irq_prio(USB_IRQn, action->prio);
	log_ep = action->channel & ~0x80;

	if( action->channel & 0x80 ){
		if( (action->handler.callback == 0) && (action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE) ){
			usb_local.write_pending &= ~(1<<log_ep);
			mcu_execute_event_handler(&(usb_local.write[log_ep]), MCU_EVENT_FLAG_CANCELED, 0);
		}
	} else {
		if( (action->handler.callback == 0) && (action->o_events & MCU_EVENT_FLAG_DATA_READY) ){
			usb_local.read_ready |= (1<<log_ep);
			mcu_execute_event_handler(&(usb_local.read[log_ep]), MCU_EVENT_FLAG_CANCELED, 0);
		}
	}



	if ( (log_ep < DEV_USB_LOGICAL_ENDPOINT_COUNT)  ){
		if( action->o_events & MCU_EVENT_FLAG_DATA_READY ){
			//cortexm_enable_interrupts(NULL);
			if( cortexm_validate_callback(action->handler.callback) < 0 ){
				return -1;
			}

			usb_local.read[log_ep].callback = action->handler.callback;
			usb_local.read[log_ep].context = action->handler.context;
			ret = 0;
		}

		if( action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){
			if( cortexm_validate_callback(action->handler.callback) < 0 ){
				return -1;
			}

			usb_local.write[log_ep].callback = action->handler.callback;
			usb_local.write[log_ep].context = action->handler.context;
			ret = 0;
		}
	}

	if( ret < 0 ){
		errno = EINVAL;
	}
	return ret;
}

int mcu_usb_configure(const devfs_handle_t * handle, void * ctl){
	usb_cfg(handle, (int)ctl);
	return 0;
}

int mcu_usb_setaddr(const devfs_handle_t * handle, void * ctl){
	usb_set_addr(handle, (u32)ctl);
	return 0;
}

int mcu_usb_resetep(const devfs_handle_t * handle, void * ctl){
	usb_reset_ep(handle, (u32)ctl);
	return 0;
}

int mcu_usb_enableep(const devfs_handle_t * handle, void * ctl){
	usb_enable_ep(handle, (u32)ctl);
	return 0;
}

int mcu_usb_disableep(const devfs_handle_t * handle, void * ctl){
	usb_disable_ep(handle, (u32)ctl);
	return 0;
}

int mcu_usb_stallep(const devfs_handle_t * handle, void * ctl){
	int ep = (int)ctl;
	usb_set_stall_ep(handle, ep);
	return 0;
}

int mcu_usb_unstallep(const devfs_handle_t * handle, void * ctl){
	int ep = (int)ctl;
	usb_clr_stall_ep(handle, ep);
	return 0;
}

int mcu_usb_cfgep(const devfs_handle_t * handle, void * ctl){
	usb_cfg_ep(handle, ctl);
	return 0;
}

int mcu_usb_seteventhandler(const devfs_handle_t * handle, void * ctl){
	memcpy(&(usb_local.special_event_handler), ctl, sizeof(mcu_event_handler_t));
	return 0;
}

int mcu_usb_dev_read(const devfs_handle_t * handle, devfs_async_t * rop){
	int ret;
	int loc = rop->loc;

	if ( loc > (DEV_USB_LOGICAL_ENDPOINT_COUNT-1) ){
		errno = EINVAL;
		return -1;
	}

	if( usb_local.read[loc].callback != NULL ){
		errno = EAGAIN;
		return -1;
	}

	//Synchronous read (only if data is ready) otherwise 0 is returned
	if ( usb_local.read_ready & (1<<loc) ){
		usb_local.read_ready &= ~(1<<loc);  //clear the read ready bit
		ret = mcu_usb_rd_ep(0, loc, rop->buf);
	} else {
		rop->nbyte = 0;
		if ( !(rop->flags & O_NONBLOCK) ){
			//If this is a blocking call, set the callback and context
			if( cortexm_validate_callback(rop->handler.callback) < 0 ){
				return -1;
			}

			usb_local.read[loc].callback = rop->handler.callback;
			usb_local.read[loc].context = rop->handler.context;
			ret = 0;
		} else {
			errno = EAGAIN;
			ret = -1;
		}
	}

	return ret;
}

int mcu_usb_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	//Asynchronous write
	int ep;
	int loc = wop->loc;


	ep = (loc & 0x7F);

	if ( ep > (DEV_USB_LOGICAL_ENDPOINT_COUNT-1) ){
		errno = EINVAL;
		return -1;
	}

	if ( usb_local.write[ep].callback != NULL ){
		errno = EAGAIN;
		return -1;
	}

	usb_local.write_pending |= (1<<ep);

	if( cortexm_validate_callback(wop->handler.callback) < 0 ){
		return -1;
	}

	usb_local.write[ep].callback = wop->handler.callback;
	usb_local.write[ep].context = wop->handler.context;

	wop->nbyte = mcu_usb_wr_ep(0, loc, wop->buf, wop->nbyte);

	if ( wop->nbyte < 0 ){
		usb_disable_ep(handle, loc );
		usb_reset_ep(handle, loc );
		usb_enable_ep(handle, loc );
		usb_local.write_pending &= ~(1<<ep);
		return -2;
	}

	return 0;
}

u32 calc_ep_addr (u32 endpoint_num){
	u32 val;
	val = (endpoint_num & 0x0F) << 1;
	if (endpoint_num & 0x80){
		val += 1;
	}
	return (val);
}

int mcu_usb_reset(const devfs_handle_t * handle, void * cfg){

	//Set max packet size of phy ep 0
	LPC_USB->EpInd = 0;
	LPC_USB->MaxPSize = mcu_board_config.usb_max_packet_zero;

	//set max packet size of phy ep 1
	LPC_USB->EpInd = 1;
	LPC_USB->MaxPSize = mcu_board_config.usb_max_packet_zero;

	while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);

	LPC_USB->EpIntClr  = 0xFFFFFFFF;
	LPC_USB->EpIntEn   = 0xFFFFFFFF ^ USB_DMA_EP;
	LPC_USB->DevIntClr = 0xFFFFFFFF;
	LPC_USB->DevIntEn = usb_irq_mask;

	return 0;
}

void usb_wakeup(int port){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_DEV_STAT, USB_SIE_DAT_WR_BYTE(DEV_CON));
}

void usb_set_addr(const devfs_handle_t * handle, u32 addr){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_ADDR, USB_SIE_DAT_WR_BYTE(DEV_EN | addr)); // Don't wait for next
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_ADDR, USB_SIE_DAT_WR_BYTE(DEV_EN | addr)); //  Setup Status Phase
}

void usb_cfg(const devfs_handle_t * handle, u32 cfg){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_CFG_DEV, USB_SIE_DAT_WR_BYTE(cfg ? CONF_DVICE : 0));
}

void usb_cfg_ep(const devfs_handle_t * handle, void * ep_desc){
	u32 num;
	mcu_usbd_endpoint_descriptor_t * ep_ptr;
	ep_ptr = (mcu_usbd_endpoint_descriptor_t*)ep_desc;
	num = calc_ep_addr(ep_ptr->bEndpointAddress);
	LPC_USB->ReEp |= (1 << num);
	LPC_USB->EpInd = num;
	LPC_USB->MaxPSize = ep_ptr->wMaxPacketSize;
	while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);
	LPC_USB->DevIntClr = EP_RLZED_INT;
}

void usb_enable_ep(const devfs_handle_t * handle, u32 endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
}

void usb_disable_ep(const devfs_handle_t * handle, u32 endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(EP_STAT_DA));
}

void usb_reset_ep(const devfs_handle_t * handle, u32 endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
}

void usb_set_stall_ep(const devfs_handle_t * handle, u32 endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(EP_STAT_ST));
}

void usb_clr_stall_ep(const devfs_handle_t * handle, u32 endpoint_num){
	usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
}

int mcu_usb_isconnected(const devfs_handle_t * handle, void * ctl){
	return usb_local.connected;
}

void usb_clr_ep_buf(const devfs_handle_t * handle, u32 endpoint_num){
	usb_sie_wr_cmd_ep(endpoint_num, USB_SIE_CMD_CLR_BUF);
}

int mcu_usb_rd_ep(const devfs_handle_t * handle, u32 endpoint_num, void * dest){
	u32 n;
	u32 * ptr;
	u32 size;
	u32 mask;

	LPC_USB->Ctrl = ((endpoint_num & 0x0F) << 2) | CTRL_RD_EN;

	//There needs to be a delay between the time USBCtrl is set and reading USBRxPLen due to the difference in clocks
	if ( mcu_core_getclock() > 48000000 ){
		int i;
		for(i=0; i < mcu_core_getclock()/4000000; i++){
			asm volatile("nop");
		}
	}

	ptr = (u32*)dest;
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
int mcu_usb_wr_ep(const devfs_handle_t * handle, u32 endpoint_num, const void * src, u32 size){
	u32 n;
	u32 * ptr = (u32*)src;

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
void mcu_core_usb0_isr(){
	u32 device_interrupt_status;
	u32 tmp;
	int i;

	device_interrupt_status = LPC_USB->DevIntSt;     //Device interrupt status
	if (device_interrupt_status & ERR_INT){ //Error interrupt
		tmp = usb_sie_rd_cmd_dat(USB_SIE_CMD_RD_ERR_STAT);
		mcu_execute_event_handler(&(usb_local.special_event_handler), MCU_EVENT_FLAG_ERROR, 0);
		LPC_USB->DevIntClr = ERR_INT;
	}

	if (device_interrupt_status & FRAME_INT){ //start of frame
		mcu_execute_event_handler(&(usb_local.special_event_handler), MCU_EVENT_FLAG_SOF, 0);
		LPC_USB->DevIntClr = FRAME_INT;
	}

	if (device_interrupt_status & DEV_STAT_INT){ //Status interrupt (Reset, suspend/resume or connect)
		cortexm_delay_us(100);
		tmp = usb_sie_rd_cmd_dat(USB_SIE_CMD_GET_DEV_STAT);
		if (tmp & DEV_RST){
			//mcu_usb_reset(0, NULL);
			usb_local.connected = 1;
			usb_local.write_pending = 0;
			usb_local.read_ready = 0;
			mcu_execute_event_handler(&(usb_local.special_event_handler), MCU_EVENT_FLAG_RESET, 0);
		}

		if ( tmp == 0x0D ){
			usb_local.connected = 0;
			for(i = 1; i < DEV_USB_LOGICAL_ENDPOINT_COUNT; i++){
				mcu_execute_event_handler(&(usb_local.read[i]), MCU_EVENT_FLAG_CANCELED, 0);
				mcu_execute_event_handler(&(usb_local.write[i]), MCU_EVENT_FLAG_CANCELED, 0);
			}
		}

		if (tmp & DEV_CON_CH){
			mcu_execute_event_handler(&(usb_local.special_event_handler), MCU_EVENT_FLAG_POWER, 0);
		}

		if (tmp & DEV_SUS_CH){
			if (tmp & DEV_SUS){
				usb_local.connected = 0;
				mcu_execute_event_handler(&(usb_local.special_event_handler), MCU_EVENT_FLAG_SUSPEND, 0);
			} else {
				usb_local.connected = 1;
				mcu_execute_event_handler(&(usb_local.special_event_handler), MCU_EVENT_FLAG_RESUME, 0);
			}

		}

		LPC_USB->DevIntClr = DEV_STAT_INT|CCEMTY_INT|CDFULL_INT|RxENDPKT_INT|TxENDPKT_INT|EP_RLZED_INT;
		return;
	}

	if (device_interrupt_status & EP_SLOW_INT){ //Slow endpoint interrupt
		slow_ep_int();
	}

	LPC_USB->DevIntClr = CCEMTY_INT|CDFULL_INT|RxENDPKT_INT|TxENDPKT_INT|EP_RLZED_INT;
	return;
}

void slow_ep_int(){
	u32 episr;
	u32 phy_ep, log_ep;
	u32 tmp;
	episr = LPC_USB->EpIntSt;
	usb_event_t event;


	for (phy_ep = 0; phy_ep < USB_EP_NUM; phy_ep++){

		if (episr & (1 << phy_ep)){
			//dstr("i:"); dhex(phy_ep); dstr("\n");
			//Calculate the logical endpoint value (associated with the USB Spec)
			log_ep = phy_ep >> 1;
			event.epnum = log_ep;

			//Clear the interrupt
			LPC_USB->EpIntClr = (1 << phy_ep);

			//Read the SIE command data
			while ((LPC_USB->DevIntSt & CDFULL_INT) == 0);
			tmp = LPC_USB->CmdData;

			//Check for endpoint 0
			if ((phy_ep & 1) == 0){ //These are the OUT endpoints

				//Check for a setup packet
				if ( (phy_ep == 0) && (tmp & EP_SEL_STP) ){
					mcu_execute_event_handler(&(usb_local.read[0]), MCU_EVENT_FLAG_SETUP, &event);
				} else {
					usb_local.read_ready |= (1<<log_ep);
					mcu_execute_event_handler(&(usb_local.read[log_ep]), MCU_EVENT_FLAG_DATA_READY, &event);
				}
			} else {  //These are the IN endpoints

				event.epnum |= 0x80;
				usb_local.write_pending &= ~(1<<log_ep);
				mcu_execute_event_handler(&(usb_local.write[log_ep]), MCU_EVENT_FLAG_WRITE_COMPLETE, &event);
			}
		}

	}
	LPC_USB->DevIntClr = EP_SLOW_INT;
}

#endif








