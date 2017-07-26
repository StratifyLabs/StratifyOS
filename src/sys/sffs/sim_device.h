
#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>

#define MCU_PACK __attribute__((packed))

typedef struct MCU_PACK {
	int tid /*! The calling task ID */;
	int flags /*! The flags for the open file descriptor */;
	int loc /*! The location to read or write */;
	union {
		const void * cbuf /*! Pointer to const void buffer */;
		void * buf /*! Pointer to void buffer */;
		const char * cchbuf /*! Pointer to const char buffer */;
		char * chbuf /*! Pointer to char buffer */;
	};
	int nbyte /*! The number of bytes to transfer */;
	void (*callback)(void*, const void*) /*! The function to call when the operation completes */;
	void * context /*! The first argument to \a callback */;
} devfs_async_t;

typedef struct {
	const void * dcfg /*! Pointer to device specific configuration */;
	void * state /*! Pointer to device specific state (RAM) */;
} devfs_handle_t;

typedef int (*dev_driver_open_t)(const devfs_handle_t*);
typedef int (*dev_driver_ioctl_t)(const devfs_handle_t*, int, void*);
typedef int (*dev_driver_read_t)(const devfs_handle_t*, devfs_async_t *);
typedef int (*dev_driver_write_t)(const devfs_handle_t*, devfs_async_t *);
typedef int (*dev_driver_close_t)(const devfs_handle_t*);

typedef struct MCU_PACK {
	dev_driver_open_t open /*! A pointer to the periph_open() function */;
	dev_driver_ioctl_t ioctl /*! A pointer to the periph_ioctl() function */;
	dev_driver_read_t read /*! A pointer to the periph_read() function */;
	dev_driver_write_t write /*! A pointer to the periph_write() function */;
	dev_driver_close_t close /*! A pointer to the periph_close() function */;
} dev_driver_t;


typedef struct {
	int handle;
	long loc;
	int index;
} cafs_DIR;


typedef struct {
	char name[32];
	uint8_t type /*! The device type: 'c': character device, 'b': block device, or 'B': block device with special location use */;
	dev_driver_t driver /*! The driver functions */;
	devfs_handle_t cfg /*! The configuration for the device */;
} device_t;


typedef void (*cortexm_svcall_t)(void*);

static inline void cortexm_svcall(cortexm_svcall_t call, void * args);
void cortexm_svcall(void (*call)(void*), void * args){
	call(args);
}

typedef struct {
	const void * fs;
	void * handle;
	int flags;
	int loc;
} open_file_t;

#ifdef NAME_MAX
#undef NAME_MAX
#endif

#define NAME_MAX 24


#endif /* DEVICE_H_ */
