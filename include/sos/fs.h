#ifndef SOS_FS_H
#define SOS_FS_H

#if defined __cplusplus
extern "C" {
#endif

int mkfs(const char *path);
int mount(const char *path);
int unmount(const char *path);

#if defined __cplusplus
}
#endif

#endif // SOS_FS_H
