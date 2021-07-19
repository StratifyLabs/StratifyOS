# Version 4.1.0

## New Features

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
