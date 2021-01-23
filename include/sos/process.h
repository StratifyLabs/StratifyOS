#ifndef SOS_PROCESS_H
#define SOS_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \details Launch an application from a data file system.
 *
 * The options are:
 * - APPFS_FLAG_IS_REPLACE
 * - APPFS_FLAG_IS_FLASH
 * - APPFS_FLAG_IS_STARTUP (only with APPFS_FLAG_IS_FLASH)
 * - APPFS_FLAG_IS_REPLACE
 * - APPFS_FLAG_IS_ORPHAN
 *
 * Here is an example of launching a new application:
 * \code
 * #include <caos.h>
 *
 * pid_t p;
 * int status;
 * char exec_path[PATH_MAX];
 *
 * p = launch("/home/HelloWorld", exec_path, 0, APPFS_FLAG_IS_FLASH, 0, 0, 0);
 *
 * wait(&status); //wait until hello world is done running
 * unlink(exec_path); //delete the installed image (or just leave it there to run again)
 *
 * \endcode
 *
 *
 * @param path Path to launch binary
 * @param exec_path The path to store the path where the binary is installed (e.g.
 * /app/flash/0-HelloWorld)
 * @param args Pointer to the arguments for launch
 * @param options Install options mask (flash, startup, etc)
 * @param ram_size The amount of RAM that will be allocated to stack/heap (excludes code
 * even if running from RAM), set to zero for default RAM size
 * @param update_progress Callback to show progress of install/launch
 * @param update_context Value passed to update_progress() callback
 * @param envp Null for this version
 * @return Zero on success
 */
int launch(
  const char *path,
  char *exec_path,
  const char *args,
  int options,
  int ram_size,
  int (*update_progress)(const void *, int, int),
  const void *update_context,
  char *const envp[]);

/*! \brief Install an application
 * \details This function installs an application in flash or RAM
 * @param path The source path
 * @param exec_path A destination buffer for the path to execute once installed
 * @param options Install options
 * @param ram_size The number of bytes to use for heap/stack
 * @param update_progress Callback to show progress of the install
 * @param update_context Value passed to update_progress() callback
 * @return Zero on success
 */
int install(
  const char *path,
  char *exec_path,
  int options,
  int ram_size,
  int (*update_progress)(const void *, int, int),
  const void *update_context);

#ifdef __cplusplus
}
#endif

#endif // SOS_PROCESS_H
