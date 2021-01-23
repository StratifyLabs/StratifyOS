#ifndef SOS_FS_H
#define SOS_FS_H

int mkfs(const char *path);
int mount(const char *path);
int unmount(const char *path);

#endif // SOS_FS_H
