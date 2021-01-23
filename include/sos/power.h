#ifndef SOS_POWER_H
#define SOS_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

int hibernate(int seconds);
void powerdown(int seconds);

#ifdef __cplusplus
}
#endif

#endif // SOS_POWER_H
