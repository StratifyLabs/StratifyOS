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
#include "mcu/usb.h"
#include "mcu/pio.h"
#include "usb_flags.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#include "usb_flags.h"

#if MCU_USB_PORTS > 0

#define USB_MAX_NON_ISO_SIZE 64
#define USB_SETUP_PACKET_SIZE 8

int EPActiveFlag;

volatile EP_LIST EPList[USB_EP_NUM*BUFFER_MULT];

#ifdef MCU_DEBUG
volatile int usbdev_stat;
#define USB_DEV_DEBUG(x) (usbdev_stat |= x)
#else
#define USB_DEV_DEBUG(x)
#endif

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

static void exec_readcallback(int ep, void * data);
static void exec_writecallback(int ep, void * data);


typedef struct {
	mcu_callback_t write_callback[USB_LOGIC_EP_NUM];
	mcu_callback_t read_callback[USB_LOGIC_EP_NUM];
	void * write_context[USB_LOGIC_EP_NUM];
	void * read_context[USB_LOGIC_EP_NUM];
	volatile int write_pending;
	volatile int read_ready;
	void (*event_handler)(usb_spec_event_t,int);
	uint8_t ref_count;
	uint8_t connected;
} usb_local_t;

static usb_local_t usb_local MCU_SYS_MEM;

static void clear_callbacks();
void clear_callbacks(){
	memset(usb_local.write_callback, 0, USB_LOGIC_EP_NUM * sizeof(void(*)(int)));
	memset(usb_local.read_callback, 0, USB_LOGIC_EP_NUM * sizeof(void(*)(int)));
}

#define EP_MSK_CTRL 0x0001 // Control Endpoint Logical Address Mask
#define EP_MSK_BULK 0xC924 // Bulk Endpoint Logical Address Mask
#define EP_MSK_INT  0x4492 // Interrupt Endpoint Logical Address Mask
#define EP_MSK_ISO  0x1248 // Isochronous Endpoint Logical Address Mask

static uint32_t calc_ep_addr (uint32_t endpoint_num);

void USB_EPInit( void ) {
	uint32_t *addr, data_ptr;
	uint32_t i, j = 0;

	addr = (uint32_t *)USB_EPLIST_ADDR;
	LPC_USB->EPLISTSTART = USB_EPLIST_ADDR;
	data_ptr = USB_EPDATA_ADDR;
	LPC_USB->DATABUFSTART = USB_EPDATA_PAGE;

	/* CTRL, BULK or Interrupt IN/OUT EPs, max EP size is 64 */
	/* For EP0, double buffer doesn't apply to CTRL EPs, but, EP0OUTBuf0 is
  for EP0OUT, EP0OUTBuf1 is for SETUP, EP0INBuf0 is for EP0IN, EP0INTBuf1 is
  reserved. Also note: ACTIVE bit doesn't apply to SETUP and Reserved EP buffer. */
	*addr++ = (mcu_core_usb_max_packet_zero<<16)|((uint16_t)(data_ptr>>6));
	EPList[j].buf_ptr = data_ptr;
	EPList[j++].buf_length = mcu_core_usb_max_packet_zero;
	data_ptr += mcu_core_usb_max_packet_zero;
	*addr++ = ((uint16_t)(data_ptr>>6));		/* No length field for SETUP */
	EPList[j].buf_ptr = data_ptr;
	EPList[j++].buf_length = USB_SETUP_PACKET_SIZE;
	/* For easier alignment, the data field is aligned to 64 bytes for all EPs. */
	data_ptr += mcu_core_usb_max_packet_zero;

	*addr++ = (mcu_core_usb_max_packet_zero<<16)|((uint16_t)(data_ptr>>6));
	EPList[j].buf_ptr = data_ptr;
	EPList[j++].buf_length = mcu_core_usb_max_packet_zero;
	data_ptr += mcu_core_usb_max_packet_zero;
#if 0
	/* EP0 IN second buffer(buffer1) is reserved. */
	*addr++ = (mcu_core_usb_max_packet_zero<<16)|((uint16_t)(data_ptr>>6));
#else
	addr++;
#endif
	EPList[j].buf_ptr = data_ptr;
	EPList[j++].buf_length = mcu_core_usb_max_packet_zero;
	data_ptr += mcu_core_usb_max_packet_zero;

	for ( i = 2; i < USB_EP_NUM; i++ )
	{
		/* Buffer 0 */
#if USB_ISO_COMMUNICATION
		*addr++ = EP_DISABLED|EP_ISO_TYPE|(USB_MAX_ISO_SIZE<<16)|((uint16_t)(data_ptr>>6));
		EPList[j].buf_ptr = data_ptr;
		EPList[j++].buf_length = USB_MAX_ISO_SIZE;
		data_ptr += USB_MAX_ISO_SIZE;
#else
		*addr++ = EP_DISABLED|(USB_MAX_NON_ISO_SIZE<<16)|((uint16_t)(data_ptr>>6));
		EPList[j].buf_ptr = data_ptr;
		EPList[j++].buf_length = USB_MAX_NON_ISO_SIZE;
		data_ptr += USB_MAX_NON_ISO_SIZE;
#endif
		/* Buffer 1 */
#if USB_ISO_COMMUNICATION
		*addr++ = EP_DISABLED|EP_ISO_TYPE|(USB_MAX_ISO_SIZE<<16)|((uint16_t)(data_ptr>>6));
		EPList[j].buf_ptr = data_ptr;
		EPList[j++].buf_length = USB_MAX_ISO_SIZE;
		data_ptr += USB_MAX_ISO_SIZE;
#else
		*addr++ = EP_DISABLED|(USB_MAX_NON_ISO_SIZE<<16)|((uint16_t)(data_ptr>>6));
		EPList[j].buf_ptr = data_ptr;
		EPList[j++].buf_length = USB_MAX_NON_ISO_SIZE;
		data_ptr += USB_MAX_NON_ISO_SIZE;
#endif
	}
	return;
}


uint32_t USB_GetEPCmdStatusPtr( uint32_t endpoint_num ) {
	uint32_t addr;

	addr = (uint32_t)USB_EPLIST_ADDR;
	addr += ((calc_ep_addr(endpoint_num)*2)<<2);	/* Based on EPCommand/Status List, all EPs are
									double buffered except CTRL EP 0, size is 32-bit. */
	return (addr);
}


void _mcu_usb_dev_power_on(int port){
	if ( usb_local.ref_count == 0 ){
		//Set callbacks to NULL
		usb_local.connected = 0;
		clear_callbacks();
		usb_local.event_handler = NULL;
		_mcu_lpc_core_enable_pwr(PCUSB_REG);
		//LPC_USB->USBClkCtrl = 0x12; //turn on dev clk en and AHB clk en
		//while( LPC_USB->USBClkCtrl != 0x12 ){}  //wait for clocks

	}
	usb_local.ref_count++;
}

void _mcu_usb_dev_power_off(int port){
	if ( usb_local.ref_count > 0 ){
		if ( usb_local.ref_count == 1 ){
			_mcu_core_priv_disable_irq((void*)(USB_IRQ_IRQn));  //Enable the USB interrupt
			//LPC_USB->USBClkCtrl = 0x0; //turn off dev clk en and AHB clk en
			//while( LPC_USB->USBClkCtrl != 0 ){}
			_mcu_lpc_core_disable_pwr(PCUSB_REG);
		}
		usb_local.ref_count--;
	}
}

int _mcu_usb_dev_powered_on(int port){
	if( _mcu_lpc_core_pwr_enabled(PCUSB_REG) ){
		return 1;
	}
	return 0;
}


int mcu_usb_getattr(int port, void * ctl){
	memcpy(ctl, &usb_ctl, sizeof(usb_attr_t));
	return 0;
}

int mcu_usb_setattr(int port, void * ctl){
	usb_attr_t * ctlp;

	ctlp = (usb_attr_t *)ctl;

#ifdef LPCXX7X_8X
	switch(ctlp->pin_assign){
	case 0:
		break;
	case 1:
		break;
	default:
		errno = EINVAL;
		return -1 - offsetof(usb_attr_t, pin_assign);
	}
#else
	if ( ctlp->pin_assign != 0 ){
		errno = EINVAL;
		return -1 - offsetof(usb_attr_t, pin_assign);
	}
#endif

	if (ctlp->mode != USB_ATTR_MODE_DEVICE){
		errno = EINVAL;
		return -1 - offsetof(usb_attr_t, mode);
	}

	//Start the USB clock
	_mcu_core_setusbclock(ctlp->crystal_freq);

	memcpy(&usb_ctl, ctl, sizeof(usb_attr_t));

	usb_local.read_ready = 0;
	usb_local.write_pending = 0;

	//Configure the IO

	_mcu_usb_cfg_pio(0, 0, 29, 30, 1, 30);

	_mcu_core_priv_enable_irq((void*)USB_IRQ_IRQn);  //Enable the USB interrupt
	mcu_usb_reset(0, NULL);
	usb_set_addr(0,0);
	return 0;
}

void usb_connect(uint32_t con){
	//usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_DEV_STAT, USB_SIE_DAT_WR_BYTE(con ? DEV_CON : 0));
	if ( con ){
		LPC_USB->DEVCMDSTAT |= USB_DCON;
	} else {
		LPC_USB->DEVCMDSTAT &= ~USB_DCON;
	}
}

int mcu_usb_attach(int port, void * ctl){
	usb_connect(1);
	return 0;
}

int mcu_usb_detach(int port, void * ctl){
	usb_connect(0);
	return 0;
}

int mcu_usb_setaction(int port, void * ctl){
	usb_action_t * action = (usb_action_t*)ctl;
	int log_ep;

	if( action->channel & 0x80 ){
		if( action->callback == 0 ){
			exec_writecallback(action->channel & ~0x80, DEVICE_OP_CANCELLED);
		}
	} else {
		if( action->callback == 0 ){
			exec_readcallback(action->channel & ~0x80, DEVICE_OP_CANCELLED);
		}
	}

	log_ep = action->channel & ~0x80;
	if ( (log_ep < USB_LOGIC_EP_NUM)  ){
		if( action->event == USB_EVENT_DATA_READY ){
			_mcu_core_priv_enable_interrupts(NULL);
			usb_local.read_callback[ log_ep ] = action->callback;
			usb_local.read_context[ log_ep ] = action->context;
			return 0;
		} else if( action->event == USB_EVENT_WRITE_COMPLETE ){
			usb_local.write_callback[ log_ep ] = action->callback;
			usb_local.write_context[ log_ep ] = action->context;
			return 0;
		}
	}

	errno = EINVAL;
	return -1;
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

int mcu_usb_seteventhandler(int port, void * ctl){
	usb_local.event_handler = (void(*)(usb_spec_event_t,int))ctl;
	return 0;
}

int _mcu_usb_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int ret;
	int loc = rop->loc;

	if ( loc > (USB_LOGIC_EP_NUM-1) ){
		errno = EINVAL;
		return -1;
	}

	if( usb_local.read_callback[loc] != NULL ){
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
			usb_local.read_callback[loc] = rop->callback;
			usb_local.read_context[loc] = rop->context;
			ret = 0;
		} else {
			errno = EAGAIN;
			ret = -1;
		}
	}

	return ret;
}

int _mcu_usb_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	//Asynchronous write
	int ep;
	int port;
	int loc = wop->loc;

	port = DEVICE_GET_PORT(cfg);

	ep = (loc & 0x7F);

	if ( ep > (USB_LOGIC_EP_NUM-1) ){
		errno = EINVAL;
		return -1;
	}

	if ( usb_local.write_callback[ep] != NULL ){
		errno = EAGAIN;
		return -1;
	}

	usb_local.write_pending |= (1<<ep);
	usb_local.write_callback[ep] = wop->callback;
	usb_local.write_context[ep] = wop->context;
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

int mcu_usb_reset(int port, void * cfg){

	//BufferUsed = 0;
	EPActiveFlag = 0;

	/* Initialize EP Command/Status List. */
	USB_EPInit();

	LPC_USB->EPINUSE = 0;
	LPC_USB->EPSKIP = 0;

	LPC_USB->DEVCMDSTAT |= USB_EN;
	/* Clear all EP interrupts, device status, and SOF interrupts. */
	LPC_USB->INTSTAT = 0xC00003FF;
	/* Enable all ten(10) EPs interrupts including EP0, note: EP won't be
	  ready until it's configured/enabled when device sending SetEPStatus command
	  to the command engine. */
	LPC_USB->INTEN  = DEV_STAT_INT | 0x3FF |  FRAME_INT;

	return 0;
}

void usb_wakeup(int port){
	LPC_USB->DEVCMDSTAT &= ~USB_DSUS;
}

void usb_set_addr(int port, uint32_t addr){
	LPC_USB->DEVCMDSTAT &= ~0x7F;
	LPC_USB->DEVCMDSTAT |= (USB_EN | addr);
}

void usb_cfg(int port, uint32_t cfg){
	if ( cfg ) {
		LPC_USB->INTEN  = DEV_STAT_INT | 0x3FF | FRAME_INT;
	} else {
		LPC_USB->INTEN  = DEV_STAT_INT | 0x03 | FRAME_INT;
	}
}

void usb_cfg_ep(int port, void * ep_desc){
	//mcu_usb_ep_desc_t * ep_ptr;
	//ep_ptr = (mcu_usb_ep_desc_t*)ep_desc;
	//num = calc_ep_addr(ep_ptr->bEndpointAddress);
	//LPC_USB->ReEp |= (1 << num);
	//LPC_USB->EpInd = num;
	//LPC_USB->MaxPSize = ep_ptr->wMaxPacketSize;
	//while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);
	//LPC_USB->DevIntClr = EP_RLZED_INT;
}

void usb_enable_ep(int port, uint32_t endpoint_num){
	//usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));
	uint32_t *addr;
	uint32_t ep_addr = calc_ep_addr(endpoint_num);

	addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
	*addr &= ~EP_DISABLED;
	if ( endpoint_num & 0x0F )	/* Non-zero EPs */
	{
		if ( endpoint_num & 0x80 )		/* For non-zero IN */
		{
			if ( LPC_USB->EPBUFCFG & (0x1U << ep_addr) )
			{
				/* For non-zero double buffer EPs, clear EP_DISABLED to both buffer. */
				addr++;
				*addr &= ~EP_DISABLED;
			}
		}
		else
		{
			/* For non-zero EP OUT, in addition to clear EP_DISABLED bits,
		  set the ACTIVE bit indicating that EP is ready to read. For
		  double buffered EPs, set ACTIVE bit and clear EP_DISABLED bit
		  for both buffer0 and 1. */
			*addr |= BUF_ACTIVE;
			if ( LPC_USB->EPBUFCFG & (0x1U << ep_addr) )
			{
				/* For double buffer. */
				addr++;
				*addr &= ~EP_DISABLED;
				*addr |= BUF_ACTIVE;
			}
		}
	}

}


void usb_disable_ep(int port, uint32_t endpoint_num){
	//usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(EP_STAT_DA));
	uint32_t epbit;

	epbit = 0x1 << calc_ep_addr(endpoint_num);
	LPC_USB->EPSKIP |= epbit;
	while ( LPC_USB->EPSKIP & epbit );
	LPC_USB->INTSTAT = epbit;			/* Clear EP interrupt(s). */

}

void usb_reset_ep(int port, uint32_t endpoint_num){
	//usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));

	uint32_t *addr;

	addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
	/* Based on EPInUse register to decide which buffer needs to toggle
	  reset. When this happens, the STALL bits need to be cleared for both
	  buffer 0 and 1. */
	*addr &= ~EP_STALL;
	*(addr+2) &= ~EP_STALL;
	if ( LPC_USB->EPINUSE & (0x1<<calc_ep_addr(endpoint_num)) ){
		addr++;
	}
	*addr |= EP_RESET;
}

void usb_set_stall_ep(int port, uint32_t endpoint_num){
	//usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(EP_STAT_ST));

	uint32_t *addr;

	if ( (endpoint_num & 0x0F) == 0 )
	{
		/* For EP0 IN or OUT, simply clear the ACTIVE and set the STALL bit. */
		addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
		/* STALL bit can't be set until ACTIVE bit is gone. */
		if ( *addr & BUF_ACTIVE )
		{
			usb_disable_ep(port, endpoint_num);
		}
		*addr |= EP_STALL;
	}
	else
	{
		/* For non-zero EPs, deactivate the EP first, make buffer inactive
		before setting the STALL bit. It applies to both buffer 0 and 1 if
		double buffer is enabled. */
		addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
		if ( *addr & BUF_ACTIVE )
		{
			usb_disable_ep(port, endpoint_num);
		}
		*addr |= EP_STALL;		/* STALL on buffer 0. */
#if USE_DOUBLE_BUFFER
		if ( LPC_USB->EPBUFCFG & (0x1U << calc_ep_addr(endpoint_num)) )
		{
			/* If double buffer is enabled, STALL on buffer 1. */
			addr++;
			if ( *addr & BUF_ACTIVE )
			{
				usb_disable_ep (endpoint_num);
			}
			*addr |= EP_STALL;
		}
#endif
	}
}

void usb_clr_stall_ep(int port, uint32_t endpoint_num){
	//usb_sie_wr_cmd_dat(USB_SIE_CMD_SET_EP_STAT(calc_ep_addr(endpoint_num)), USB_SIE_DAT_WR_BYTE(0));

	uint32_t *addr;
	uint32_t ep_addr = calc_ep_addr(endpoint_num);

	if ( (endpoint_num & 0x0F) == 0 )
	{
		/* For EP0 IN and OUT, simply clear the STALL bit. */
		addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
		*addr &= ~EP_STALL;
	}
	else
	{
		addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
		/* For non-zero EPs if double buffer, both STALL bits should be cleared. */
		*addr &= ~EP_STALL;
		*(addr+1) &= ~EP_STALL;

		/* Based on EPInUse register to decide which buffer needs to toggle reset. */
		if ( LPC_USB->EPINUSE & (0x1<<ep_addr) )
		{
			/* Buffer 1 is in use. Toggle Reset Buffer 1, otherwise, toggle Reset
		  buffer 0. */
			addr++;
		}
		*addr |= EP_RESET;
		if ( !(endpoint_num & 0x80) )
		{
			/* For non-zero EP OUT, ACTIVE bit and length field need to
		  be set again after clearing STALL. */
			*addr &= ~(PKT_LNGTH_MASK<<16);
#if USB_ISO_COMMUNICATION
			*addr |= ((USB_MAX_ISO_SIZE<<16)| BUF_ACTIVE);
#else
			*addr |= ((USB_MAX_NON_ISO_SIZE<<16)| BUF_ACTIVE);
#endif
		}
		else
		{
			/* For non-zero EP IN, the EPActiveFlag will be set when WriteEP() happens
		  while STALL is set. If so, when ClearSTALL happens, set the ACTIVE bit that
		  data buffer is ready to write data to the EPs. */
			if ( EPActiveFlag & (0x1U << ep_addr) )
			{
				*addr |= BUF_ACTIVE;
				EPActiveFlag &= ~(0x1U << ep_addr);
			}
		}
	}

}

int mcu_usb_isconnected(int port, void * ctl){
	return usb_local.connected;
}

void usb_clr_ep_buf(int port, uint32_t endpoint_num){
	//usb_sie_wr_cmd_ep(endpoint_num, USB_SIE_CMD_CLR_BUF);
}

int mcu_usb_rd_setup_ep(int port, uint32_t endpoint_num, void * dest){
	uint32_t cnt, n;
	uint32_t *addr;
	uint32_t *dataptr;
	uint8_t * pData = dest;

	addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );

	/* Check/Clear STALL on both EP0 IN and OUT when SETUP is received. */
	if ( (*addr & EP_STALL) || ((*addr+2) & EP_STALL) )
	{
		*addr &= ~EP_STALL;
		*(addr+2) &= ~EP_STALL;
	}

	cnt = USB_SETUP_PACKET_SIZE;
	dataptr = (uint32_t *)EPList[1].buf_ptr;
	for (n = 0; n < (cnt + 3) / 4; n++) {
		*((__packed uint32_t *)pData) = *((__packed uint32_t *)dataptr);
		pData += 4;
		dataptr++;
	}

	addr++;		/* Use EP0 buffer 1 for SETUP packet */
	/* Fixed Command/Status location(EPList[1] for SETUP. Reset buffer pointer
	  field, SETUP length is fixed with eight bytes. */
	*addr &= ~0x3FFFFFF;
	*addr |= (uint16_t)((EPList[1].buf_ptr)>>6);
	return (cnt);

}

int mcu_usb_rd_ep(int port, uint32_t endpoint_num, void * dest){
	uint32_t cnt, n;
	uint32_t *addr;
	uint32_t *dataptr;
	uint32_t index = 0;
	uint8_t * pData = dest;
	uint32_t ep_addr = calc_ep_addr(endpoint_num);

	addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
	index = ep_addr * BUFFER_MULT;		/* Double buffer is considered. */
#if USE_DOUBLE_BUFFER
	if ( endpoint_num & 0x0F ){
		/* For EP0 IN/OUT, there is no double buffer. For non-zero
		EP, double buffer is considered. If BufferUsed bit mask is set,
		buffer0 is used, switch to buffer 1, index needs to be changed
		accordingly too. */
		if ( BufferUsed & (0x1U << ep_addr) )
		{
			addr++;		/* Use buffer 1 */
			index++;
		}
	}
#endif

	cnt = (*addr >> 16) & PKT_LNGTH_MASK;
	/* The NBytes field decrements by H/W with the packet byte each time. */
	cnt = EPList[index].buf_length - cnt;
	dataptr = (uint32_t *)EPList[index].buf_ptr;
	for (n = 0; n < (cnt + 3) / 4; n++) {
		*((__packed uint32_t *)pData) = *((__packed uint32_t *)dataptr);
		pData += 4;
		dataptr++;
	}

	/* Clear buffer after EP read, reset EP length and buffer pointer field */
	*addr &= ~0x3FFFFFF;
	if ( (endpoint_num & 0x0F) == 0 )
	{
		/* EP0 is single buffer only. */
		*addr |= ((EPList[index].buf_length<<16)
				| (uint16_t)((EPList[index].buf_ptr)>>6) | BUF_ACTIVE);
	}
	else
	{
		/* Toggle buffer if double buffer is used for non-zero EPs.  */
#if USE_DOUBLE_BUFFER
		if ( LPC_USB->EPBUFCFG & (0x1U << ep_addr) ){
			BufferUsed = LPC_USB->EPINUSE;
			if ( BufferUsed & (0x1U << ep_addr) )
			{
				addr++;		/* Set buffer 1 ACTIVE */
				index++;
			}
			else
			{
				addr--;		/* Set buffer 0 ACTIVE */
				index--;
			}
		}
#endif
		*addr |= ((EPList[index].buf_length<<16)
				| (uint16_t)((EPList[index].buf_ptr)>>6) | BUF_ACTIVE);
	}
	return (cnt);
}


/*! \details
 */
int mcu_usb_wr_ep(int port, uint32_t endpoint_num, const void * src, uint32_t size){
	uint32_t n, index;
	uint32_t *addr;
	uint32_t *dataptr;
	const uint8_t * pData = src;
	uint32_t cnt = size;
	uint32_t ep_addr = calc_ep_addr(endpoint_num);

	addr = (uint32_t *)USB_GetEPCmdStatusPtr( endpoint_num );
	index = calc_ep_addr(endpoint_num) * 2;		/* Double buffer is considered. */
	if ( !(endpoint_num & 0x0F) )
	{
		/* When EP0 IN is received, set ACTIVE bit on both EP0 IN
		and OUT. */
		*(addr-2) |= BUF_ACTIVE;	/* Set ACTIVE bit on EP0 OUT */
	}
	else
	{
		/* For non-zero EPs, if double buffer is used and EPInUse is set, buffer0
		is used, otherwise, buffer1 is used. */
		if ( LPC_USB->EPBUFCFG & (0x1U << ep_addr) )
		{
			if ( LPC_USB->EPINUSE & (0x1U << ep_addr) )
			{
				addr++;		/* move to buffer1 address in EP command/status list. */
				index++;
			}
		}
	}

	/* Get EP command/status List, update the length field and data pointer. */
	*addr &= ~0x3FFFFFF;
	cnt &= PKT_LNGTH_MASK;
	*addr |= (cnt<<16)|(uint16_t)((EPList[index].buf_ptr)>>6);

	dataptr = (uint32_t *)EPList[index].buf_ptr;
	/* Stuff the data first, whether send out or not(set ACTIVE bit) is based
	  on STALL condition. */
	for (n = 0; n < (cnt + 3) / 4; n++) {
		*((__packed uint32_t *)dataptr) = *((__packed uint32_t *)pData);
		pData += 4;
		dataptr++;
	}

	if ( (*addr & EP_STALL) && (endpoint_num & 0x0F) )
	{
		/* This is for MSC class when STALL occurs and non-zero EPs,
		set the ACTIVE flag, but don't do anything until ClearFeature
		to clear STALL, then tranfer the data. */
		EPActiveFlag |= (0x1U << ep_addr);
		return (cnt);
	}
	*addr |= BUF_ACTIVE;
	return (cnt);
}


/*! \details This function services the USB interrupt request.
 */
void _mcu_core_usb_isr1(){
	uint32_t device_interrupt_status;
	uint32_t val;

	USB_DEV_DEBUG(0x01);
	device_interrupt_status = LPC_USB->INTSTAT;     //Device interrupt status
	LPC_USB->INTSTAT = device_interrupt_status;

	if (device_interrupt_status & FRAME_INT){ //start of frame
		USB_DEV_DEBUG(0x04);
		if ( usb_local.event_handler ){
			usb_local.event_handler(USB_SPEC_EVENT_SOF, 0);
		}
	}

	if( device_interrupt_status & DEV_STAT_INT ){
		val = LPC_USB->DEVCMDSTAT;

		if( val & USB_DRESET_C ){ //reset
			LPC_USB->DEVCMDSTAT |= USB_DRESET_C;

			USB_DEV_DEBUG(0x10);
			//mcu_usb_reset(0, NULL);
			usb_local.connected = 1;
			usb_local.write_pending = 0;
			usb_local.read_ready = 0;
			if ( usb_local.event_handler ){
				usb_local.event_handler(USB_SPEC_EVENT_RESET, 0);
			}
		}

		if( val & USB_DSUS_C ){ //suspend
			LPC_USB->DEVCMDSTAT |= USB_DSUS_C;

			if (val & USB_DSUS){
				usb_local.connected = 0;
				if ( usb_local.event_handler ){
					usb_local.event_handler(USB_SPEC_EVENT_SUSPEND, val);
				}
			} else {
				usb_local.connected = 1;
				if ( usb_local.event_handler ){
					usb_local.event_handler(USB_SPEC_EVENT_RESUME, val);
				}
			}
		}


		if( val & USB_DCON_C ){
			LPC_USB->DEVCMDSTAT |= USB_DCON_C;

			if ( usb_local.event_handler ){
				usb_local.event_handler(USB_SPEC_EVENT_POWER, val);
			}
		}

		/*
		if (device_interrupt_status & DEV_STAT_INT){ //Status interrupt (Reset, suspend/resume or connect)
			USB_DEV_DEBUG(0x08);
			_delay_us(100);
			tmp = usb_sie_rd_cmd_dat(USB_SIE_CMD_GET_DEV_STAT);
			if (tmp & DEV_RST){
				USB_DEV_DEBUG(0x10);
				//mcu_usb_reset(0, NULL);
				usb_local.connected = 1;
				usb_local.write_pending = 0;
				usb_local.read_ready = 0;
				if ( usb_local.event_handler ){
					usb_local.event_handler(USB_SPEC_EVENT_RESET, 0);
				}
			}

			if ( tmp == 0x0D ){
				USB_DEV_DEBUG(0x20);
				usb_local.connected = 0;
				for(i = 1; i < USB_LOGIC_EP_NUM; i++){
					if( usb_local.read_callback[i] != NULL ){
						if( usb_local.read_callback[i](usb_local.read_context[i], (const void*)-1) == 0 ){
							usb_local.read_callback[i] = NULL;
						}
					}

					if( usb_local.write_callback[i] != NULL ){
						if( usb_local.write_callback[i](usb_local.write_context[i], (const void*)-1) == 0 ){
							usb_local.write_callback[i] = NULL;
						}
					}

				}
			}

			if (tmp & DEV_CON_CH){
				if ( usb_local.event_handler ){
					usb_local.event_handler(USB_SPEC_EVENT_POWER, tmp);
				}
			}

			if (tmp & DEV_SUS_CH){
				if (tmp & DEV_SUS){
					usb_local.connected = 0;
					if ( usb_local.event_handler ){
						usb_local.event_handler(USB_SPEC_EVENT_SUSPEND, tmp);
					}
				} else {
					usb_local.connected = 1;
					if ( usb_local.event_handler ){
						usb_local.event_handler(USB_SPEC_EVENT_RESUME, tmp);
					}
				}

			}

			//LPC_USB->INTSTAT = DEV_STAT_INT|CCEMTY_INT|CDFULL_INT|RxENDPKT_INT|TxENDPKT_INT|EP_RLZED_INT;
			return;
		}
		 */
	}

	if (device_interrupt_status & 0x3FF){ //Slow endpoint interrupt
		slow_ep_int(device_interrupt_status & 0x3FF);
	}

	return;
}

void slow_ep_int(int episr){
	uint32_t phy_ep, log_ep;

	USB_DEV_DEBUG(0x100);
	for (phy_ep = 0; phy_ep < USB_EP_NUM; phy_ep++){
		if (episr & (1 << phy_ep)){
			//Calculate the logical endpoint value (associated with the USB Spec)
			log_ep = phy_ep >> 1;

			//Check for endpoint 0
			if ((phy_ep & 1) == 0){ //These are the OUT endpoints

				//Check for a setup packet
				if ( (phy_ep == 0) && (LPC_USB->DEVCMDSTAT & USB_SETUP_RCVD) ){
					USB_DEV_DEBUG(0x200);
					if (usb_local.read_callback[0]){
						if( usb_local.read_callback[0](usb_local.read_context, (const void*)USB_SETUP_EVENT) == 0 ){
							usb_local.read_callback[0] = NULL;
						}
					}
					LPC_USB->DEVCMDSTAT |= USB_SETUP_RCVD;
				} else {
					exec_readcallback(log_ep, (void*)USB_OUT_EVENT);

				}
			} else {  //These are the IN endpoints
				exec_writecallback(log_ep, (void*)USB_IN_EVENT);
			}
		}

	}
	//LPC_USB->DevIntClr = EP_SLOW_INT;
}

static void exec_readcallback(int log_ep, void * data){
	usb_local.read_ready |= (1<<log_ep);
	if( usb_local.read_callback[log_ep] != NULL ){
		if( usb_local.read_callback[log_ep](usb_local.read_context[log_ep], data) == 0 ){
			usb_local.read_callback[log_ep] = NULL;
		}
	}
}

static void exec_writecallback(int log_ep, void * data){
	usb_local.write_pending &= ~(1<<log_ep);
	if( usb_local.write_callback[log_ep] != NULL ){
		if( usb_local.write_callback[log_ep](usb_local.write_context[log_ep], data) == 0 ){
			usb_local.write_callback[log_ep] = NULL;
		}
	}
}

#endif








