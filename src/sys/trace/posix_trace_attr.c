// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include <errno.h>
#include <string.h>

#include "trace.h"

int posix_trace_attr_destroy(trace_attr_t *attr) { return 0; }

int posix_trace_attr_getclockres(const trace_attr_t *attr, struct timespec *clockres) {
  return 0;
}

int posix_trace_attr_getcreatetime(
  const trace_attr_t *attr,
  struct timespec *createtime) {
  createtime->tv_sec = attr->create_time.tv_sec;
  createtime->tv_nsec = attr->create_time.tv_nsec;
  return 0;
}

int posix_trace_attr_getgenversion(const trace_attr_t *attr, char *genversion) {
  return 0;
}

int posix_trace_attr_getinherited(const trace_attr_t *attr, int *what) { return 0; }

int posix_trace_attr_getlogfullpolicy(const trace_attr_t *attr, int *what) {
  *what = attr->log_policy;
  return 0;
}
int posix_trace_attr_getlogsize(const trace_attr_t *attr, size_t *size) {
  // write maximum log size to size from attr
  *size = attr->log_size;
  return 0;
}

int posix_trace_attr_getmaxdatasize(const trace_attr_t *attr, size_t *size) {
  *size = attr->data_size;
  return 0;
}

int posix_trace_attr_getmaxsystemeventsize(const trace_attr_t *attr, size_t *size) {
  *size = attr->data_size + sizeof(struct posix_trace_event_info);
  return 0;
}

int posix_trace_attr_getmaxusereventsize(
  const trace_attr_t *attr,
  size_t data_len,
  size_t *event_size) {
  *event_size = data_len + sizeof(struct posix_trace_event_info);
  return 0;
}

int posix_trace_attr_getname(const trace_attr_t *attr, char *name) {
  strncpy(name, attr->name, APPFS_NAME_MAX);
  return 0;
}

int posix_trace_attr_getstreamfullpolicy(const trace_attr_t *attr, int *policy) {
  *policy = attr->stream_policy;
  return 0;
}

int posix_trace_attr_getstreamsize(const trace_attr_t *attr, size_t *size) {
  *size = attr->stream_size;
  return 0;
}

int posix_trace_attr_init(trace_attr_t *attr) {
  attr->data_size = 32;
  attr->log_policy = POSIX_TRACE_APPEND;
  attr->stream_policy = POSIX_TRACE_LOOP;
  attr->stream_size = 8;
  strcpy(attr->name, "trace-stream");
  attr->log_size = 1024 * 10;

  return 0;
}

int posix_trace_attr_setinherited(trace_attr_t *attr, int what) {
  errno = ENOTSUP;
  return -1;
}

int posix_trace_attr_setlogfullpolicy(trace_attr_t *attr, int policy) {
  attr->log_policy = policy;
  return 0;
}

int posix_trace_attr_setlogsize(trace_attr_t *attr, size_t size) {
  // set the maximum log size
  // check limits
  attr->log_size = size;
  return 0;
}

int posix_trace_attr_setmaxdatasize(trace_attr_t *attr, size_t size) {
  attr->data_size = size;
  return 0;
}

int posix_trace_attr_setname(trace_attr_t *attr, const char *name) {
  memset(attr->name, 0, NAME_MAX + 1);
  strncpy(attr->name, name, NAME_MAX);
  return 0;
}

int posix_trace_attr_setstreamsize(trace_attr_t *attr, size_t size) {
  attr->stream_size = size;
  return 0;
}

int posix_trace_attr_setstreamfullpolicy(trace_attr_t *attr, int policy) {
  attr->stream_policy = policy;
  return 0;
}
