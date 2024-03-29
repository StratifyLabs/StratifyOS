#include <sys/cdefs.h>
// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup FAULT Fault Handling
 * @{
 *
 * \ingroup CORE
 *
 * \details The fault handling module handles processor faults.  When a fault occurs, the
 * fault handler decodes the fault and calls scheduler_fault_event_handler() which must be
 * provided externally by the application.
 *
 */

/*! \file
 * \brief Fault Handling Header file (Defines Fault Codes)
 */

#ifndef CORTEXM_FAULT_H_
#define CORTEXM_FAULT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sdk/types.h>

/*! \details This lists the possible values of a cortexm fault.
 *
 */
typedef enum {
  MCU_FAULT_NONE,
  MCU_FAULT_MEMORY_STACKING /*! 1 A fault occurred while stacking */,
  MCU_FAULT_MEMORY_UNSTACKING /*! 2 A fault occured while unstacking */,
  MCU_FAULT_MEMORY_ACCESS_VIOLATION /*! 3 A read or write was attempted on a location
                                       where this is not allowed */
  ,
  MCU_FAULT_MEMORY_EXEC_VIOLATION /*! 4 An instruction fetch was attempted on memory that
                                     is marked as non-executable */
  ,
  MCU_FAULT_BUS_STACKING /*! 5 A bus fault occurred while stacking */,
  MCU_FAULT_BUS_UNSTACKING /*! 6 A bus fault occurred whild unstacking */,
  MCU_FAULT_BUS_IMPRECISE /*! 7 An imprecise bus fault occurred */,
  MCU_FAULT_BUS_PRECISE /*! 8 A precision bus fault occurred */,
  MCU_FAULT_BUS_INSTRUCTION /*! 9 A bus fault occurred when fetching an instruction */,
  MCU_FAULT_USAGE_DIVBYZERO /*! A A divide by zero was attempted */,
  MCU_FAULT_USAGE_UNALIGNED /*! B An unaligned memory access */,
  MCU_FAULT_USAGE_NO_COPROCESSOR /*! C An instruction attempted to access a coprocessor
                                    that is not supported */
  ,
  MCU_FAULT_USAGE_INVALID_PC /*! D In invalid PC was loaded on return */,
  MCU_FAULT_USAGE_INVALID_STATE /*! E An instruction has made invalid use of the EPSR
                                   register */
  ,
  MCU_FAULT_USAGE_UNDEFINED_INSTRUCTION /*! F The processor has attempted to execute an
                                           undefined instruction */
  ,
  MCU_FAULT_HARD_VECTOR_TABLE /*! 10 This is a bus fault on a vector table read */,
  MCU_FAULT_UNHANDLED_INTERRUPT /*! 11 This is executed if a interrupt request routine is
                                   not defined */
  ,
  MCU_FAULT_MEMORY_FP_LAZY /*! 12 Floating point lazy state preservation fault */,
  MCU_FAULT_HARD_UNKNOWN /*! 13 This is an unknown fault */,
  MCU_FAULT_BUS_UNKNOWN /*! 14 An unknown bus fault */,
  MCU_FAULT_MEM_UNKNOWN /*! 15 An unknown memory fault */,
  MCU_FAULT_USAGE_UNKNOWN /*! 16 An unknown usage fault */,
  MCU_FAULT_WDT /*! 17 A WDT interrupt caused the fault */,
  MCU_FAULT_TOTAL
} fault_num_t;

typedef struct {
  int num /*! The Fault number (see \ref fault_num_t) */;
  void *addr /*! The faulting address if applicable */;
  void *pc /*! The thread program counter */;
  void *caller /*! The thread caller */;
  void *handler_pc /*! The handler program counter */;
  void *handler_caller /*! The handler caller */;
} fault_t;

int cortexm_fault_init() MCU_ROOT_CODE;

// weak so that bootloader can provide thin alternates
void cortexm_hardfault_handler() MCU_ROOT_EXEC_CODE;
void cortexm_memfault_handler() MCU_ROOT_EXEC_CODE;
void cortexm_busfault_handler() MCU_ROOT_EXEC_CODE;
void cortexm_usagefault_handler() MCU_ROOT_EXEC_CODE;

/*! \details This function must be provided by the application or
 * OS to handle faults.
 */
extern void cortexm_fault_event_handler(fault_t *fault) MCU_ROOT_EXEC_CODE;

#ifdef __cplusplus
}
#endif

#endif /* CORTEXM_FAULT_H_ */

/*! @} */
