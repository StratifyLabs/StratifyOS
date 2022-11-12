#ifndef SOS_TRACE_H
#define SOS_TRACE_H

#include "link/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SOS_TRACE_MESSAGE(msg)                                                           \
  sos_trace_event(LINK_POSIX_TRACE_MESSAGE, msg, sizeof(msg))
#define SOS_TRACE_WARNING(msg)                                                           \
  sos_trace_event(LINK_POSIX_TRACE_WARNING, msg, sizeof(msg))
#define SOS_TRACE_CRITICAL(msg)                                                          \
  sos_trace_event(                                                                       \
    LINK_POSIX_TRACE_CRITICAL, msg, sizeof(msg))
#define SOS_TRACE_FATAL(msg)                                                             \
  sos_trace_event(LINK_POSIX_TRACE_FATAL, msg, sizeof(msg))

void sos_trace_event(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len);
int sos_trace_stack(u32 count);
int sos_trace_stack_with_pointer(
  const u32 *first_link_register,
  const u32 *sp,
  const u32 *stack_top,
  u32 count);
void sos_trace_event_addr_tid(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len,
  u32 addr,
  int tid);
void sos_trace_root_trace_event(
  link_trace_event_id_t event_id,
  const void *data_ptr,
  size_t data_len);

#ifdef __cplusplus
}
#endif

#endif // SOS_TRACE_H
