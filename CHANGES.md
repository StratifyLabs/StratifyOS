# Version 4.1

- M7 CPUs use MPU regions 9 to 15. This leaves lower priority regions 0 to 8 for user use.
- Added additional cache policies
  - You will need to update your `sos_config.cache` settings to add policies for external flash and tightly coupled memories
  - You also need to update `sos_config.cache` to indicate how OS code/data memories should be treated
- Fixed MPU cache settings for tasks
- Invalidate cache when deleting flash blocks in appfs
- Cleanup of device/fifo to remove warnings
- Implement FIFO_FLAG_INIT (not just I_FIFO_INIT)


# Version 4.0

- Revamped cmake system compared to 3.x
- Changed to source available license (requires registering for a free hardware ID)
- Changed how board support packages are integrated as discussed in the [porting guide](guides/Porting.md)
