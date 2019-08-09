/* Copyright 2011-2018 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/*! \addtogroup AUTH Authorization device
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This driver provides details about the currently installed operating system.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_AUTH_H_
#define SOS_DEV_AUTH_H_

#include "mcu/types.h"
#include "sos/link/types.h"


#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_VERSION (0x030000)
#define AUTH_IOC_CHAR 'A'

typedef struct MCU_PACK {
	u8 data[32];
} auth_token_t;

typedef struct MCU_PACK {
	auth_token_t key;
	auth_token_t token;
} auth_key_token_t;

#define I_AUTH_GETVERSION _IOCTL(AUTH_IOC_CHAR, 0)

/*! \brief See below for details.
 * \details This provides a random
 * token exchange. The caller provides
 * a 128-bit random number token (left justified)
 *
 * The auth device populates the remaining
 * 128-bits of the random number. The result
 * is used with the secret key I_AUTH_FINISH.
 *
 */
#define I_AUTH_START _IOCTLRW(AUTH_IOC_CHAR, 1, auth_token_t)


/*! \brief See below for details.
 * \details This will calculate the final
 * token if the caller already has the secret key.
 *
 * Storing the secret key inside an unencrypted application
 * binary is a big security risk.
 *
 * They key and randome number are passed. The final token is returned.
 *
 *
 *
 */
#define I_AUTH_GETTOKEN _IOCTLRW(AUTH_IOC_CHAR, 1, auth_key_token_t)


/*! \brief See below for details.
 * \details This finishes the authentication.
 *
 * Once the calling thread is authenticated, it will
 * be placed in root access mode.
 *
 * Executing I_AUTH_FINISH with an invalid value will
 * remove root access mode from the calling thread.
 *
 * The system must set the SYS_FLAG_IS_KEYED flag and appened a secret key to
 * the end of the binary. Without both of these, all calls
 * to I_AUTH will result in the calling thread having root access.
 *
 * The authorization goes like this:
 *
 * ```
 * #include <unistd.h>
 * #include <fcntl.h>
 * #include <sos/dev/auth.h>
 *
 *
 * int main(int argc, char * argv[]){
 *   int fd = open("/dev/auth", O_RDWR);
 *
 *   auth_token_t token;
 *
 *   //populate token.data[0 to 15] with random bits
 *
 *   ioctl(fdm, I_AUTH_START, &auth);
 *
 *   //all 256 bits of auth.token are now populated
 *   //This token can be sent to an external program to populate next auth token
 *   //the next auth token should be the SHA256 of the secret key and random bits token
 *
 *	  //if this program has access to the secret key -- the next token can be calculated using:
 *   auth_key_token_t key_token;
 *   //poplate key_token.key;
 *   key_token.token = token;
 *   ioctl(fd, I_AUTH_GETTOKEN, &key_token);
 *
 *   if( ioctl(fd, I_AUTH_FINISH, &auth) == 0 ){
 *     //calling task now has root privileges
 *   }
 *
 *   //now final auth token is SHA256 of the random token and secret key
 *   //this can be sent to the authorizing entity to validate the /auth/dev driver
 *
 *  if( ioctl(fd, I_AUTH_START, 0) == 0 ){
 *     //calling task no longer has root privileges
 *   }
 * }
 * ```
 *
 */
#define I_AUTH_FINISH _IOCTLRW(AUTH_IOC_CHAR, 2, auth_token_t)


#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_SYS_H_ */

/*! @} */
