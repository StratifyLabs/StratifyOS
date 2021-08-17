# Version 4.1.1

## Bug Fixes

- device fifo needs to return 0 on ioctl calls it handles

# Version 4.1.0

## New Features

- Update appfs execution check to support backward compatible binaries
- Add `MPU_MEMORY_NULL` so all valid MPU memory types are non-zero
- Add `sos_config.mcu.task_mpu_region_offset` to config defaults to zero and is OK for MCU's with only 8 regions. Set to 8 for MCUs with 16 regions
- Use `SOS_EVENT_ROOT_FATAL` for all check config errors (look at the debug output if the system doesn't boot)
- Add defaults to `config.h`
- Removed all per-process timer code when `CONFIG_TASK_PROCESS_TIMER_COUNT` is zero
- Add `pthread_mutex_timedlock()` to symbol table
- Implement `PTHREAD_INHERIT_SCHED` when creating new threads
- If an application is signed, it can start as authenticated without the parent being authenticated
- Update symbols table signature to have a base of `0x00000410`
- Add `getppid()` to symbols list
- Added `auth_flash` device for installing signed OS using a full instance bootloader
- Appfs updates
  - Add support for `I_APPFS_GETVERSION` -- returns 0x400
  - Add `I_APPFS_IS_SIGNATURE_REQUIRED` -- returns 1 if signature is required, 0 otherwise
  - Add `I_APPFS_VERIFY_SIGNATURE` to send the signature of the installed image
    - The signature is verified using the devices public keys and appropriate limitations are imposed on the installed application
- Add `get_public_key()` to `sos_sys_config_t` to allow the system to provide public keys (can be NULL)
- Add mechanisms to allow the appfs to have a list of allowed public keys
- `/dev/auth` can provide the public key from the bootloader using `I_AUTH_GET_PUBLIC_KEY`
- Bootloader version bumped to `0x400` which handles new requests
  - `I_BOOTLOADER_IS_SIGNATURE_REQUIRED`
  - `I_BOOTLOADER_GET_PUBLIC_KEY`
  - `I_BOOTLOADER_VERIFY_SIGNATURE`
- `link` functions added for bootloader signature and verification
- `CONFIG_BOOT_IS_VERIFY_SIGNATURE` is used in the user `sos_config.h` file to turn signature verification on and off
  - User must provide api's for requests: `CRYPT_SHA256_ROOT_API_REQUEST` and `CRYPT_ECC_ROOT_API_REQUEST` (just set key and verify)
- Bootloader can authenticate with a secret key and verify an image with a public key
- Add encryption and authentication calls to bootloader API
- Add link3 transport protocol using secure transmissions
- Add support for low level cryptography hooks in `sos_config`.
- Add `sos_config.h` inclusion which must be provided by the CMake super project
  - This allows fine tuning of the OS
- Added additional cache policies
  - You will need to update your `sos_config.cache` settings to add policies for external flash and tightly coupled memories
  - You also need to update `sos_config.cache` to indicate how OS code/data memories should be treated
- M7 CPUs use MPU regions 9 to 15. This leaves lower priority regions 0 to 8 for user use.
- Fixed MPU cache settings for tasks

## Bug Fixes

- Fixed bug with `I_SYS_GETID` where too many chars were copied on some platforms
- Fixed bug with CRT where atexit() was not initialized correctly causing crashes when the application exited
- `kill()` now always sends to the primary task (non-thread) of the specified pid
- Return error (`EINVAL`) if attempting to set a priority ceiling that is invalid
- Ensure priority elevation within SVCALL setting to prevent inadvertent priority elevation
- Fixed vulnerability when starting authenticated apps
- Fix assignment of `st_ino` in devfs when using `stat` and `fstat`
- Implement `I_SYS_GETVERSION` on `/dev/sys`
- Fixed a major bug with `realloc` (it was basically unusable) (51b1840a30f69886d18185d747b1c88368b7a513)
- Remove `MCU_PACK` from `ffifo_state_t` to avoid potential issues with unaligned access (90d879d6c6f42e4a754b9cd662e31aa4fe2e01a6)
- Fix APPFS define for ram usage calculation (0634fd751eb49243981813769ac59232f7c2f674)
- Make LWIP path headers PUBLIC for mcu target (d9208179a12cc02ae71563e246a7225f02aba0de)
- Emit a fatal event on bootup if system memory is too small (1cb40bb)
- Add a short delay in `link_thread.c` when `link_slaveread` fails to account for fast USB drivers (7114032)
- Invalidate cache when deleting flash blocks in appfs (cae2798)
- Cleanup of device/fifo to remove warnings
- Implement FIFO_FLAG_INIT (not just I_FIFO_INIT) (42f2783)

# Version 4.0.0

- Revamped cmake system compared to 3.x
- Changed to source available license (requires registering for a free hardware ID)
- Changed how board support packages are integrated as discussed in the [porting guide](guides/Porting.md)
