# Device Driver Guide

On Stratify OS, hardware is accessed through the filesystem using the POSIX system calls: [open(), close(), read(), write(), ioctl()](/reference/StratifyOS/unistd/).

## Accessing Hardware on Stratify OS

The following POSIX calls are used to access hardware on Stratify OS.

```c
#include <unistd.h>
int open(const char * name, int flags, int mode);
int close(int fd);
int read(int fd, void * buf, int nbyte);
int write(int fd, const void * buf, int nbyte);
int ioctl(int fd, int request, ...);
```

### Opening a Device

All devices are located in the "/dev" folder and are named based on their function plus a port number (custom devices don't necessarily follow this, but MCU peripherals do).

For example, to open the uart, we use the following code.

```c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int fd;
fd = open("/dev/uart0", O_RDWR);
if( fd < 0 ){
    perror("failed to open uart0");
}
```

If "/dev/uart0" can be opened, fd (zero or more) will be associated with uart0. Once a device is opened, it needs to be configured (unless it has already been configured by another process or thread).

### Configuring a Device

The application can 

 - configure the entire device including the pin mapping (not portable), 
 - configure the device with the default pin mapping, 
 - or configure the device using all default settings.

The default pin mapping and settings are provided by the board support package (BSP). If the BSP doesn't include defaults and the application specifies the defaults, the ioctl() call will return an error.

To see an example of how the BSP defines the default pins and settings, please see the [Nucleo-F446ZE board configuration file](https://github.com/StratifyLabs/Nucleo-F446ZE/blob/master/src/board_config.c) (look for the i2c_config_t as an example).

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/uart.h> //declares uart_attr_t and I_UART_SETATTR

ioctl(fd, I_UART_SETATTR); //this will use all BSP defaults

uart_attr_t attr;
memset(&attr.pin_assignment, 0xff, sizeof(uart_pin_assignment_t));
attr.o_flags = UART_FLAG_SET_LINE_CODING | UART_FLAG_IS_STOP1 | UART_FLAG_IS_PARITY_NONE;
attr.width = 8;
attr.freq = 115200;
ioctl(fd, I_UART_SETATTR, &attr); //this will use above settings plus BSP default pins

uart_attr_t attr;
memset(&attr.pin_assignment, 0xff, sizeof(uart_pin_assignment_t));
attr.o_flags = UART_FLAG_SET_LINE_CODING | UART_FLAG_IS_STOP1 | UART_FLAG_IS_PARITY_NONE;
attr.width = 8;
attr.freq = 115200;
attr.pin_assignment.tx = mcu_pin(0,0);
attr.pin_assignment.rx = mcu_pin(0,1);
attr.pin_assignment.cts = mcu_pin(0xff,0xff); //don't use CTS
attr.pin_assignment.rts = mcu_pin(0xff,0xff); //don't use RTS
ioctl(fd, I_UART_SETATTR, &attr); //this will use above settings and pin mapping
````

### Getting Device Info and Version

In addition to the I_NAME_SETATTR request, all MCU peripherals will respond to the following calls.

- I_NAME_GETVERSION
- I_NAME_GETINFO

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/uart.h> //declares uart_attr_t and I_UART_SETATTR

int version;
uart_info_t info;
ioctl(fd, I_UART_GETINFO, &info);

printf("Flags 0x%lX\n", info.o_flags); //these flags are supported on the hardware when using I_UART_SETATTR for the uart_attr_t.o_flags

version = ioctl(fd, I_UART_GETVERSION);
if( (version & ~0xffff) != (UART_VERSION & ~0xffff) ){ //the minor and patch version can be different
    //there is a major driver version mismatch -- won't work correctly
    printf("The application is using a different driver than the BSP\n");
}
````

### Reading and Writing a Device

Reading and writing a device is done just like any other file using read() and write(). Operations on devices can be either synchronous or asynchronous. If they are synchronous, they can be blocking or non-blocking.

- Asynchronous: returns immediately and read/write happens in the background
- Synchronous: returns when complete
  - Blocking: complete when data has been read (at least 1 byte) or written
  - Non-blocking: complete when data has been read or written or when no data is available to be read or written

The file descriptor (when opened) determines whether calls to read() and write() are blocking or non-blocking.  

A non-blocking call will return immediately if no data is available. For example, if you read() the UART and the UART has no data, the function returns right away. A blocking call will wait until data becomes available.

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/uart.h> //declares uart_attr_t and I_UART_SETATTR

char buffer[16];
int ret;
int fd = open("/dev/uart0", O_RDWR); //blocking
ioctl(fd, I_UART_SETATTR);
ret = read(fd, buffer, 16); //this will return when 1 or more bytes arrive on the UART
printf("Read %d bytes\n", ret);
if( ret > 0 ){ //echo the bytes
    write(fd, buffer, ret);
}
close(fd);
````

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/uart.h> //declares uart_attr_t and I_UART_SETATTR

char buffer[16];
int ret;
int fd = open("/dev/uart0", O_RDWR | O_NONBLOCK); //non-blocking
ioctl(fd, I_UART_SETATTR);
ret = read(fd, buffer, 16); //this will return immediately
if( ret < 0 ){
    perror("No data"); //errno is set to EAGAIN
} else {
    printf("Got %d bytes\n", ret);
    write(fd, buffer, ret); //this is "non-blocking" but thread will block until complete
}
close(fd);
````

When data is being transferred, synchronous calls will yield the processor. That is to say if you write 100 bytes to the UART at 9600bps that will take about 100ms. The calling thread will allow other threads to execute and the OS will use interrupts to send the data in the background.

Asynchronous calls are done using aio_read() and aio_write(). These calls return immediately.
````c 
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/uart.h> //declares uart_attr_t and I_UART_SETATTR
#include <aio.h>

char buffer[16];
int fd = open("/dev/uart0", O_RDWR); //non blocking doesn't matter for AIO calls
struct aiocb *aiocbp;
aiocbp.aio_fildes = fd;
aiocbp.aio_offset = 0;
aiocbp.aio_buf = buffer;
aiocbp.aio_nbytes = 16;
aio_read(&aiocbp);
while( aio_error(&aiocbp) == EINPROGRESS ){
    usleep(1000); //wait for operation to complete
}
close(fd);
````

### Location to Read or Write

 Devices are accessed like files, but they don't necessarily behave like them. Devices are classified as:

- Character: location is not updated on read/write and has no meaning
- Special Character: location is not updated on read/write, but it has meaning when set using lseek()
- Block: location is updated on read/write

All POSIX file descriptors keep track of the location or offset in a file. For character devices, this is typically ignored. For block devices (like files), it is used to specify the address and is auto-updated on read write. The position can be set manually using the following code.

````c
#include <unistd.h>
#include <stdio.h>

lseek(fd, 0, SEEK_SET); //set offset to zero
printf("Offset is %d\n", lseek(fd, 0, SEEK_CURRENT)); //this returns the current position without changing it
````

For character devices, the location is ignored. For special character devices, the location determines the channel. The following devices have channels (not an exhaustive list).

- USB: channel is the endpoint
- ADC: channel is the ADC input channel
- PWM: channel is the PWM output channel
- DAC: channel is the DAC output channel
- I2C: location determines register pointer (in some modes)

These devices don't have channels and are pure character devices (location can be used with lseek() but is meaningless).

- UART
- SPI
- PIO (also called GPIO)

These are block devices where the location is auto-updated on read/write.

- EEPROM: location determines memory address to read/write
- MEM: location determines memory address
- FLASH: location determines memory address
- DISK (abstraction for external flash devices): location is block or memory address
- SDCARD: location is memory address

The following code shows how the ADC (special character device) behaves when reading.

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/adc.h>

int fd = open("/dev/adc0", O_RDWR);
u32 samples[16];
ioctl(fd, I_ADC_SETATTR); //default BSP settings and pins
lseek(fd, 4, SEEK_SET); //seek to ADC channel 4
read(fd, samples, 16*sizeof(u32)); //read 16 samples on channel 4
read(fd, samples, 16*sizeof(u32)); //read 16 samples on channel 4 -- offset stays the same
lseek(fd, 2, SEEK_SET); //seek to ADC channel 2
read(fd, samples, 16*sizeof(u32)); //read 16 samples on channel 2
close(fd);
````

The EEPROM is a block device and updates the offset automatically.

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/eeprom.h>

int fd = open("/dev/eeprom0", O_RDWR);
eeprom_info_t info;
int i;

ioctl(fd, I_EEPROM_SETATTR); 
ioctl(fd, I_EEPROM_GETINFO, &info);
lseek(fd, 0, SEEK_SET); //start at eeprom address 0
for(i=0; i < info.size; i++){ //this loop will read the entire EEPROM
    read(&c, 1);
    printf("EEPROM[%d] = 0x%X\n", i, c);
}
close(fd);
````

### Closing Devices

When closing a device, Stratify OS both release the file description and may also shut off the hardware. If close() is called while other file descriptors are still open, Stratify OS will leave the device enabled. If close() is called and there are no other referencing file descriptors, the device will be powered off (PIO--aka GPIO--devices are an exception to this behavior).

For example, the PWM output will stop oscillating and power off when all PWM devices are closed as illustrated below.

````c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sos/dev/pwm.h>

int i;
int fd = open("/dev/pwm0", O_RDWR);
ioctl(fd, I_PWM_SETATTR); //default BSP settings and pins - determines period and freq

mcu_channel_t channel;
channel.loc = 0; //channel 0
channel.value = 500;
ioctl(fd, I_PWM_SETCHANNEL, &channel); //set channel to 500
ioctl(fd, I_PWM_ENABLE); //start the PWM

//PWM is on
for(i=0; i < 1000; i++){
    usleep(1000); //wait for 1ms
}

close(fd); //this will shutdown the PWM and channel 0
````

Keep in mind, when the program exits (return from main()), close() will be called on all open file descriptors.

### Stratify API hal namespace: C++ Classes for Hardware Access

The [Stratify API hal namespace](../StratifyAPI/#namespace-hal) provides C++ classes for accessing MCU peripherals as well as general devices.

The classes provide a method for open(), close(), read(), write(), ioctl() plus a method for each ioctl request. For example, the PWM code above becomes:

````c++
#include <sapi/hal.hpp> //uses namespace hal -- includes sos/dev/pwm.h

int i;
Pwm pwm(0);

pwm.open();
pwm.set_attr();
pwm.set_channel(0, 500);
pwm.enable();
for(i=0; i < 1000; i++){
    usleep(1000);
}
pwm.close():
````



