// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md



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

