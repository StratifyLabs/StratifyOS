// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_FS_TYPES_H_
#define SOS_FS_TYPES_H_

#include <sdk/types.h>

#define DEVFS_NAME_MAX 23
#define APPFS_NAME_MAX 23
#define APPFS_ID_MAX 23
#define ASSETFS_NAME_MAX 255

#if !defined __link
typedef open_file_t sysfs_file_t;
#endif

enum sysfs_user { SYSFS_ROOT = 0, SYSFS_USER = 1, SYSFS_GROUP = 0 };

typedef struct {
  u32 port /*! The port associated with the device (for mcu peripherals) */;
  const void *config /*! Pointer to device configuration (flash) */;
  void *state /*! \brief Pointer to device state (RAM) */;
} devfs_handle_t;

typedef struct {
  int tid /*! The calling task ID */;
  int flags /*! The flags for the open file descriptor */;
  int loc /*! The location to read or write */;
  union {
    const void *buf_const /*! Pointer to const void buffer */;
    void *buf /*! Pointer to void buffer */;
  };
  int nbyte;
  int result;
  mcu_event_handler_t handler /*! The function to call when the operation completes */;
} devfs_async_t;

typedef int (*devfs_open_t)(const devfs_handle_t *);
typedef int (*devfs_ioctl_t)(const devfs_handle_t *, int, void *);
typedef int (*devfs_read_t)(const devfs_handle_t *, devfs_async_t *);
typedef int (*devfs_write_t)(const devfs_handle_t *, devfs_async_t *);
typedef int (*devfs_close_t)(const devfs_handle_t *);

typedef struct {
  devfs_open_t open;
  devfs_ioctl_t ioctl;
  devfs_read_t read;
  devfs_write_t write;
  devfs_close_t close;
} devfs_driver_t;

typedef struct {
  devfs_async_t *read;  // used with read operations
  devfs_async_t *write; // used with write operations
} devfs_transfer_handler_t;

// mcu_execute_read_complete(devfs_transfer_handler_t * transfer_handler);

typedef struct {
  char name[DEVFS_NAME_MAX + 1] /*! The name of the device */;
  u16 uid /*! The user ID of the device (either user or root) */;
  u16 mode /*! The file access values */;
  devfs_driver_t driver /*! \brief The driver functions */;
  devfs_handle_t handle /*! \brief The configuration for the device */;
  u32 size /*! \brief Size of the device (used with block devices) */;
} devfs_device_t;

#define DEVFS_IOC_IDENT_CHAR 'D'

typedef char devfs_get_name_t[DEVFS_NAME_MAX + 1];

#define I_DEVFS_GETNAME _IOCTLW(DEVFS_IOC_IDENT_CHAR, I_MCU_TOTAL, devfs_get_name_t)

#endif /* SOS_FS_TYPES_H_ */
