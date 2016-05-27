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


/*! \addtogroup GRP
 * @{
 *
 * \details This interface is not implemented in this version.
 *
 *
 */

/*! \file */


#include <grp.h>

static struct group default_group = {
		.gr_name = "users",
		.gr_passwd = NULL,
		.gr_gid = 0,
		.gr_mem = NULL
};

/*! \details Not implemented (stub only).
 *
 * \return default group
 */
struct group *getgrgid (gid_t gid){
	return &default_group;
}

/*! \details Not implemented (stub only).
 *
 * \return default group
 */
struct group *getgrnam (const char * name){
	return &default_group;
}

/*! \details Not implemented (stub only).
 *
 * \return 0
 */
int getgrnam_r(const char * name, struct group * grp,
			char * buffer, size_t bufsize, struct group **result){
	return 0;
}

/*! \details Not implemented (stub only).
 *
 * \return 0
 */
int getgrgid_r (gid_t gid, struct group * grp,
			char * buffer, size_t bufsize, struct group ** result){
	return 0;
}

/*! \details Not implemented (stub only).
 *
 * \return NULL
 */
struct group *getgrent (){
	return NULL;
}

/*! \details Not implemented (stub only).
 *
 */
void setgrent (){

}

/*! \details Not implemented (stub only).
 *
 */
void endgrent (){

}

/*! @} */

