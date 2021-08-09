# Version 4.0.2

## Bug Fixes

- Fix a critical bug with `I_SYS_GETID` copying too many bytes
- Implement `I_SYS_GETVERSION` in `/dev/sys`
- Zero terminate arguments when starting a new process
- Fix `st_ino` assignment in `/dev/*` when using `stat()` and `fstat()`
- Emit a fatal event if system memory is too small

# Version 4.0.1

## Bug Fixes

- Fix APPFS preprocessor define for RAM usage calcs (0634fd751eb49243981813769ac59232f7c2f674)
- Add Changes.md
- Cleanup of device/fifo to remove warnings
- Implement FIFO_FLAG_INIT (not just I_FIFO_INIT)

# Version 4.0

- Revamped cmake system compared to 3.x
- Changed to source available license (requires registering for a free hardware ID)
- Changed how board support packages are integrated as discussed in the [porting guide](guides/Porting.md)
