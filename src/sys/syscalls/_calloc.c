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


#include <reent.h>
#include <malloc.h>
#include <sys/types.h>
#include <string.h>

void * _calloc_r(struct _reent * reent_ptr, size_t s1, size_t s2){
	int size;
	void * alloc;
	size = s1*s2;
	alloc = _malloc_r(reent_ptr, size);
	if ( alloc != NULL ){
		memset(alloc, 0, size);
	}
	return alloc;
}
