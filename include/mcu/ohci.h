/*
 * ohci.h
 *
 *  Created on: Dec 2, 2011
 *      Author: tgil
 */

#ifndef OHCI_H_
#define OHCI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	  const  uint32_t HcRevision;
	  volatile uint32_t HcControl;
	  volatile uint32_t HcCommandStatus;
	  volatile uint32_t HcInterruptStatus;
	  volatile uint32_t HcInterruptEnable;
	  volatile uint32_t HcInterruptDisable;
	  volatile uint32_t HcHCCA;
	  const  uint32_t HcPeriodCurrentED;
	  volatile uint32_t HcControlHeadED;
	  volatile uint32_t HcControlCurrentED;
	  volatile uint32_t HcBulkHeadED;
	  volatile uint32_t HcBulkCurrentED;
	  const  uint32_t HcDoneHead;
	  volatile uint32_t HcFmInterval;
	  const  uint32_t HcFmRemaining;
	  const  uint32_t HcFmNumber;
	  volatile uint32_t HcPeriodicStart;
	  volatile uint32_t HcLSTreshold;
	  volatile uint32_t HcRhDescriptorA;
	  volatile uint32_t HcRhDescriptorB;
	  volatile uint32_t HcRhStatus;
	  volatile uint32_t HcRhPortStatus1;
	  volatile uint32_t HcRhPortStatus2;
} ohci_t;

typedef struct {
    volatile  uint32_t  ctrl;
    volatile  uint32_t  curr_buf_ptr;
    volatile  uint32_t  next_td;
    volatile  uint32_t  buf_end;
} ohci_td_t;

typedef struct {
    volatile  uint32_t  ctrl;
    volatile  uint32_t  tail_td;
    volatile  uint32_t  head_td;
    volatile  uint32_t  next_ed;
} ohci_ed_t;


#ifdef __cplusplus
}
#endif


#endif /* OHCI_H_ */
