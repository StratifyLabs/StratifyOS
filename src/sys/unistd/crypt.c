// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file
 */

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/syslimits.h>
#include <string.h>
#include <stdio.h>

#include "sos/fs/sysfs.h"


/*! \details Encrypts a block of data.
 *
 * \return No return value but errno may be set to:
 * - ENOSYS: function not supported on this platform
 *
 */
void encrypt(char block[64], int edflag){

}


char * crypt(const char *key, const char *salt){
    return 0;
}

void setkey(const char *key){

}


/*! @} */

