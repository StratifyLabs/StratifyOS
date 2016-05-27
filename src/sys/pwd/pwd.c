/* Copyright 2011-2016 Tyler Gilbert; 
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

#include <pwd.h>
#include <errno.h>

/*! \addtogroup PWD Password
 * @{
 *
 * \ingroup POSIX
 *
 * \details This interface is not supported in this version.
 *
 */

/*! \file */


/*! \details Not implemented (stub only).
 * \return NULL
 */
struct passwd *getpwuid (uid_t uid){
	return NULL;
}

/*! \details Not implemented (stub only).
 * \return NULL
 */
struct passwd *getpwnam (const char * name){
	return NULL;
}

/*! \details Not implemented (stub only).
 * \return 0
 */
int getpwnam_r(const char * name, struct passwd * pwd, char * buffer, size_t bufsize, struct passwd **result){
	return 0;
}

/*! \details Not implemented (stub only).
 * \return 0
 */
int getpwuid_r (uid_t uid, struct passwd * pwd, char * buffer, size_t bufsize, struct passwd **result){
	return 0;
}

/*! \details Not implemented (stub only).
 * \return NULL
 */
struct passwd *getpwent (){
	return NULL;
}

/*! \details Not implemented (stub only).
 */
void setpwent (){

}

/*! \details Not implemented (stub only).
 */
void endpwent (){

}

/*! @} */
