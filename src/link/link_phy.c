// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "link_local.h"

#define MAX_DEVICE_PATH 1024

static int check_name_length(const char *name) {
  if (strnlen(name, MAX_DEVICE_PATH) >= MAX_DEVICE_PATH) {
    return -1;
  }
  return 0;
}

#if defined __win32 || defined __win64
#include <windows.h>

typedef struct {
  HANDLE handle;
  char name[MAX_DEVICE_PATH];
} link_phy_container_t;

#define API_COM_PREFIX "/serial/"
#define API_COM_PORT_NAME "serial/COM"
#define COM_PORT_NAME "\\\\.\\COM"
#define COM_PORT_MAX 500

int link_phy_getname(char *dest, const char *last, int len) {
  int com_port = 0;
  char windows_name[24]; //\\\\.\\COM
  char api_name[24];     // serial/COM4
  int does_not_exist;

  // first find the last port
  if (strlen(last) > 0) {
    do {
      api_name[23] = 0;
      snprintf(api_name, 23, "%s%d", API_COM_PORT_NAME, com_port++);
      if (strncmp(api_name, last, 23) == 0) {
        break;
      }
    } while (com_port < COM_PORT_MAX);
  }

  does_not_exist = 1;
  while ((com_port < COM_PORT_MAX) && does_not_exist) {
    snprintf(windows_name, 23, "%s%d", COM_PORT_NAME, com_port);
    snprintf(api_name, 23, "%s%d", API_COM_PORT_NAME, com_port);

    HANDLE test_handle;
    DWORD err;

    test_handle = CreateFile(
      windows_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, 0);
    if (test_handle == INVALID_HANDLE_VALUE) {
      err = GetLastError();
      if (err == 5) { // busy
        does_not_exist = 0;
      } else {
        does_not_exist = 1;
      }
    } else {
      does_not_exist = 0;
      CloseHandle(test_handle);
    }

    com_port++;
  }

  if (com_port == COM_PORT_MAX) {
    return -1;
  }

  strncpy(dest, api_name, (u32)len);
  return 0;
}

int link_phy_status(link_transport_phy_t handle) {
  link_phy_container_t *phy = handle;
  HANDLE test_handle;
  DWORD err;

  test_handle = CreateFile(
    phy->name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
    0);
  if (test_handle == INVALID_HANDLE_VALUE) {
    err = GetLastError();
    if (err == 5) {
      // all is well
      return 0;
    }
  } else {
    CloseHandle(test_handle);
    err = 100;
  }

  return -1;
}

link_transport_phy_t link_phy_open(const char *name, const void *options) {

  link_phy_container_t *handle;
  DCB params;

  // convert from serial/ to \\.\COM

  char windows_name[64];

  const int len = strlen(API_COM_PREFIX);
  if (strncmp(name, API_COM_PREFIX, len) == 0) {
    snprintf(windows_name, 62, "\\\\.\\%s", name + len);
  } else if (strncmp(name, "COM", 3) == 0) {
    snprintf(windows_name, 62, "\\\\.\\%s", name);
  } else {
    strncpy(windows_name, name, 63);
  }

  if (check_name_length(windows_name) < 0) {
    return LINK_PHY_OPEN_ERROR;
  }

  handle = malloc(sizeof(link_phy_container_t));
  if (handle == 0) {
    return LINK_PHY_OPEN_ERROR;
  }

  memset(handle->name, 0, MAX_DEVICE_PATH);
  strncpy(handle->name, windows_name, MAX_DEVICE_PATH - 1);

  link_debug(LINK_DEBUG_INFO, "Open device %s as %s", name, windows_name);

  handle->handle = CreateFile(
    windows_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL, 0);
  if (handle->handle == INVALID_HANDLE_VALUE) {
    free(handle);
    link_debug(LINK_DEBUG_INFO, "failed to open %s", windows_name);
    return LINK_PHY_OPEN_ERROR;
  }

  link_debug(LINK_DEBUG_MESSAGE, "Set timeouts for %s", windows_name);
  COMMTIMEOUTS timeouts = {0};
  timeouts.ReadIntervalTimeout = 1;
  timeouts.ReadTotalTimeoutConstant = 1;
  timeouts.ReadTotalTimeoutMultiplier = 1;
  timeouts.WriteTotalTimeoutConstant = 0;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  if (!SetCommTimeouts(handle->handle, &timeouts)) {
    // error occurred. Inform user
    link_error("Failed to set Timeouts %d\n", GetLastError());
    CloseHandle(handle->handle);
    free(handle);
    return LINK_PHY_OPEN_ERROR;
  }

  if (!GetCommState(handle->handle, &params)) {
    CloseHandle(handle->handle);
    free(handle);
    return LINK_PHY_OPEN_ERROR;
  }

  const link_transport_serial_options_t *serial_options = options;

  // assign defaults
  memset(&params, 0, sizeof(params));
  params.BaudRate = 460800;
  params.ByteSize = 8;
  params.StopBits = ONESTOPBIT;
  params.Parity = NOPARITY;
  params.fBinary = 1;
  params.fParity = 0;

  if (serial_options) {
    link_debug(
      LINK_DEBUG_MESSAGE,
      "Use custom serial port settings %dbps, %d stop bits, %d parity",
      serial_options->baudrate, serial_options->stop_bits, serial_options->parity);
    params.BaudRate = (DWORD)serial_options->baudrate;
    if (serial_options->stop_bits == 2) {
      params.StopBits = TWOSTOPBITS;
    } else {
      params.StopBits = ONESTOPBIT;
    }

    if (serial_options->parity) {
      if (serial_options->parity % 1 == 1) {
        params.Parity = ODDPARITY;
      } else {
        params.Parity = EVENPARITY;
      }
    } else {
      params.Parity = NOPARITY;
    }
  } else {
    link_debug(
      LINK_DEBUG_MESSAGE,
      "Use default serial port settings 460800bps, 1 stop bits, 0 parity");
  }

  if (!SetCommState(handle->handle, &params)) {
    CloseHandle(handle->handle);
    free(handle);
    link_error(
      "Failed set COMM state with error %d for %s", GetLastError(), windows_name);
    return LINK_PHY_OPEN_ERROR;
  }

  // SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  return handle;
}

int link_phy_write(link_transport_phy_t handle, const void *buf, int nbyte) {
  DWORD bytes_written;
  link_phy_container_t *phy = handle;

  if (handle == LINK_PHY_OPEN_ERROR) {
    link_error("bad handle");
    return LINK_PHY_ERROR;
  }

  if (link_phy_status(handle) < 0) {
    link_error("bad handle status");
    return LINK_PHY_ERROR;
  }

  if (!WriteFile(phy->handle, buf, (DWORD)nbyte, &bytes_written, NULL)) {
    link_error("Failed to write %d bytes from handle:%p\n", nbyte, handle);
    return LINK_PHY_ERROR;
  }
  return (int)bytes_written;
}

int link_phy_read(link_transport_phy_t handle, void *buf, int nbyte) {
  DWORD bytes_read;
  link_phy_container_t *phy = handle;

  if (handle == LINK_PHY_OPEN_ERROR) {
    return LINK_PHY_ERROR;
  }

  if (link_phy_status(handle) < 0) {
    return LINK_PHY_ERROR;
  }

  if (!ReadFile(phy->handle, buf, (DWORD)nbyte, &bytes_read, NULL)) {
    link_error("Failed to read %d bytes from handle:%p\n", nbyte, handle);
    return LINK_PHY_ERROR;
  }
  return (int)bytes_read;
}

int link_phy_close(link_transport_phy_t *handle) {
  link_phy_container_t *phy = (link_phy_container_t *)*handle;

  if (phy == LINK_PHY_OPEN_ERROR) {
    return 0;
  }

  *handle = LINK_PHY_OPEN_ERROR;
  if (CloseHandle(phy->handle) == 0) {
    link_error("Failed to close handle (last error %d)", GetLastError());
  }
  free(phy);
  return 0;
}

void link_phy_wait(int msec) { SleepEx((DWORD)msec, true); }

void link_phy_flush(link_transport_phy_t handle) {
  char c;
  while (link_phy_read(handle, &c, 1) == 1) {
  }
}
#endif

#if defined __macosx || defined __linux
#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>

#define BAUDRATE 460800

typedef struct {
  int fd;
  char device_path[MAX_DEVICE_PATH];
} link_phy_container_t;

// This is the mac osx prefix -- this needs to be in a list so it can also check bluetooth
#ifdef __macosx
#define TTY_DEV_PREFIX "tty.usbmodem"
#define READDIR(a, b, c) readdir_r(a, b, c)
#endif

#ifdef __linux
#define TTY_DEV_PREFIX "ttyACM"
#define READDIR(a, b, c) read_directory(a, b, c)

int read_directory(DIR *dirp, struct dirent *entry, struct dirent **result) {
  *result = readdir(dirp);
  if (*result != 0) {
    memcpy(entry, *result, sizeof(struct dirent));
    return 0;
  }
  return -1;
}

#endif

int link_phy_getname(char *dest, const char *last, int len) {
  // lookup the next eligible device
  struct dirent entry;
  struct dirent *result;
  DIR *dirp;
  int pre_len;
  int past_last;
  char entry_path[256];

  dirp = opendir("/dev");
  if (dirp == NULL) {
    link_error("/dev directory not found");
    return LINK_PHY_ERROR;
  }

  pre_len = strlen(TTY_DEV_PREFIX);
  past_last = false;
  if (strlen(last) == 0) {
    past_last = true;
  }

  while ((READDIR(dirp, &entry, &result) == 0) && (result != NULL)) {
    if (strncmp(TTY_DEV_PREFIX, entry.d_name, pre_len) == 0) {
      // the entry matches the prefix

      snprintf(entry_path, 255, "/dev/%s", entry.d_name);

      if (past_last == true) {
        if (strlen(entry.d_name) > (size_t)len) {
          // name won't fit in destination
          closedir(dirp);
          return LINK_PHY_ERROR;
        }

        strncpy(dest, entry_path, len);
        closedir(dirp);
        return 0;
      } else if (strcmp(last, entry_path) == 0) {
        past_last = true;
      }
    }
  }

  // no more entries to be found
  closedir(dirp);
  return LINK_PHY_ERROR;
}

link_transport_phy_t link_phy_open(const char *name, const void *s_options) {

  link_transport_phy_t phy;
  int fd;
  struct termios options;
  link_phy_container_t *container;

  if (strnlen(name, MAX_DEVICE_PATH) >= MAX_DEVICE_PATH) {
    return LINK_PHY_OPEN_ERROR;
  }

  const char slash_serial_prefix[] = "/serial";
  const size_t slash_len = sizeof(slash_serial_prefix) - 1;
  if (strncmp(name, slash_serial_prefix, slash_len) == 0) {
    name = name + slash_len;
    link_debug(LINK_DEBUG_MESSAGE, "stripping /serial from beginning of name: %s", name);
  }

  const char serial_prefix_at[] = "serial@";
  const size_t at_len = sizeof(serial_prefix_at) - 1;
  if (strncmp(name, serial_prefix_at, at_len) == 0) {
    name = name + at_len;
    link_debug(LINK_DEBUG_MESSAGE, "stripping serial@ from beginning of name: %s", name);
  }

  // open serial port
  fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    link_error("Failed to open '%s' %d", name, errno);
    return LINK_PHY_OPEN_ERROR;
  }

  if (ioctl(fd, TIOCEXCL) == -1) {
    link_error("failed to make device exclusive");
    return LINK_PHY_OPEN_ERROR;
  }

  memset(&options, 0, sizeof(options));

  // make the buffer raw
  cfmakeraw(&options); // raw with no buffering

  link_debug(LINK_DEBUG_INFO, "Open %s at %d bps no parity", name, BAUDRATE);
  cfsetspeed(&options, BAUDRATE);
  // even parity
  // 8 bit data
  // one stop bit
  options.c_cflag &= ~PARENB; // parity off
  options.c_cflag &= ~PARODD; // parity is not odd (use even parity)
  options.c_cflag &= ~CSTOPB; // one stop bit
  options.c_cflag |= CREAD;   // enable receiver
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CREAD | CLOCAL | CS8; // 8 bit data
  options.c_cc[VMIN] = 0;
  options.c_cc[VTIME] = 0;

  const link_transport_serial_options_t *serial_options = s_options;

  if (serial_options) {
    link_debug(
      LINK_DEBUG_MESSAGE,
      "Use custom serial port settings %dbps, %d stop bits, %d parity",
      serial_options->baudrate, serial_options->stop_bits, serial_options->parity);
    cfsetspeed(&options, serial_options->baudrate);
    if (serial_options->stop_bits == 2) {
      options.c_cflag |= CSTOPB; // two stop bits
    } else {
      options.c_cflag &= ~CSTOPB; // one stop bit
    }

    if (serial_options->parity) {
      options.c_cflag |= PARENB; // parity on
      if (serial_options->parity % 1 == 1) {
        options.c_cflag |= PARODD; // parity is  odd
      } else {
        options.c_cflag &= ~PARODD; // parity is not odd (use even parity)
      }
    } else {
      options.c_cflag &= ~PARENB; // parity off
    }
  } else {
    link_debug(
      LINK_DEBUG_MESSAGE,
      "Use default serial port settings 460800bps, 1 stop bits, 0 parity");
  }

  if (tcflush(fd, TCIFLUSH) < 0) {
    return LINK_PHY_OPEN_ERROR;
  }

  // set the attributes
  if (tcsetattr(fd, TCSANOW, &options) < 0) {
    return LINK_PHY_OPEN_ERROR;
  }

  phy = malloc(sizeof(link_phy_container_t));
  container = phy;
  if (phy == 0) {
    close(fd);
    return LINK_PHY_OPEN_ERROR;
  }

  container->fd = fd;
  strncpy(container->device_path, name, MAX_DEVICE_PATH);

  link_phy_flush(phy);
  return phy;
}

int link_phy_status(link_transport_phy_t handle) {
  link_phy_container_t *phy = handle;

  if (access(phy->device_path, F_OK) < 0) {
    // file does not exist
    fflush(stdout);

    return LINK_PHY_ERROR;
  }
  return 0;
}

int link_phy_write(link_transport_phy_t handle, const void *buf, int nbyte) {
  link_phy_container_t *phy = handle;
  int tmp;
  int ret;
  int bytes_written = 0;
  int page_size;
  int max_page_size = 1024;
  const char *p = buf;

  if (handle == LINK_PHY_OPEN_ERROR) {
    return LINK_PHY_ERROR;
  }

  if (link_phy_status(handle) < 0) {
    return LINK_PHY_ERROR;
  }

  do {
    if (nbyte - bytes_written > max_page_size) {
      page_size = max_page_size;
    } else {
      page_size = nbyte - bytes_written;
    }

    // printf("write %d for %d of %d\n", page_size, bytes_written, nbyte);
    tmp = errno;
    ret = write(phy->fd, p, page_size);
    if (ret < 0) {
      if (errno == EAGAIN) {
        errno = tmp;
        return 0;
      }
      return LINK_PHY_ERROR;
    }

    if (page_size == max_page_size) {
      // this just seems to force the OS to flush the write because of the context change
      // the context change probably gives the MCU more than 100us
      usleep(100);
    }

    p += ret;
    bytes_written += ret;

  } while (bytes_written < nbyte);

  link_debug(LINK_DEBUG_DEBUG, "Tx'd %d bytes", ret);
  return nbyte;
}

int link_phy_read(link_transport_phy_t handle, void *buf, int nbyte) {
  int ret;
  int tmp;
  link_phy_container_t *phy = handle;

  if (handle == LINK_PHY_OPEN_ERROR) {
    return LINK_PHY_ERROR;
  }

  tmp = errno;

  if (link_phy_status(handle) < 0) {
    return LINK_PHY_ERROR;
  }

  ret = read(phy->fd, buf, nbyte);
  if (ret < 0) {
    if (errno == EAGAIN) {
      errno = tmp;
      link_phy_wait(1);
      return 0;
    }
    return LINK_PHY_ERROR;
  }

  if (ret != 0) {
    link_debug(LINK_DEBUG_DEBUG, "Rx'd %d bytes", ret);
  } else {
    link_phy_wait(1);
  }
  return ret;
}

int link_phy_close(link_transport_phy_t *handle) {
  link_phy_container_t *phy = (link_phy_container_t *)*handle;
  if (*handle == LINK_PHY_OPEN_ERROR) {
    return LINK_PHY_ERROR;
  }
  *handle = LINK_PHY_OPEN_ERROR;
  int fd = phy->fd;
  free(phy);

  if (close(fd) < 0) {
    return LINK_PHY_ERROR;
  }

  return 0;
}

void link_phy_wait(int msec) { usleep(msec * 1000); }

void link_phy_flush(link_transport_phy_t handle) {
  unsigned char c;
  while (link_phy_read(handle, &c, 1) == 1) {
    ;
  }
}

#endif

int link_phy_lock(link_transport_phy_t phy) {
  MCU_UNUSED_ARGUMENT(phy);
  return 0;
}

int link_phy_unlock(link_transport_phy_t phy) {
  MCU_UNUSED_ARGUMENT(phy);
  return 0;
}
