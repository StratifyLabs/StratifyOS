# Version 4.1.0

## New Features

- Added additional cache policies
  - You will need to update your `sos_config.cache` settings to add policies for external flash and tightly coupled memories
  - You also need to update `sos_config.cache` to indicate how OS code/data memories should be treated
- M7 CPUs use MPU regions 9 to 15. This leaves lower priority regions 0 to 8 for user use.
- Fixed MPU cache settings for tasks

## Bug Fixes

- Emit a fatal event on bootup if system memory is too small (1cb40bb)
- Add a short delay in `link_thread.c` when `link_slaveread` fails to account for fast USB drivers (7114032)
- Invalidate cache when deleting flash blocks in appfs (cae2798)
- Cleanup of device/fifo to remove warnings
- Implement FIFO_FLAG_INIT (not just I_FIFO_INIT) (42f2783)

# Version 4.0.0

- Revamped cmake system compared to 3.x
- Changed to source available license (requires registering for a free hardware ID)
- Changed how board support packages are integrated as discussed in the [porting guide](guides/Porting.md)
