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

#ifndef DEBUG_BOOTLOADER_H_
#define DEBUG_BOOTLOADER_H_

#ifdef ___debug
#include "mcu/types.h"

/*! \details This function converts x to a string (signed integer)
 * and outputs the string using the write function.
 */
int dint(int x);
/*! \details This function converts x to a string (unsigned integer)
 * and outputs the string using the write function.
 */
int duint(unsigned int x);
/*! \details This function outputs a string using the write function.
 */
int dstr(char * str);
/*! \details This function converts x to a string (hex lowercase)
 * and outputs the string using the write function.
 */
int dhex(int x);
/*! \details This function converts x to a string (hex uppercase)
 * and outputs the string using the write function.
 */
int dHex(int x);
/*! \details This function converts x to a string (binary)
 * and outputs the string using the write function.
 */
int dbin(int x);
/*! \details This function converts x to a string (floating point)
 * and outputs the string using the write function.
 */
int dfloat(float x);

/*! \details This function sets the callback to write data to
 * the debug output.
 */
void dsetwritefunc(int (*func)(const void *, int));
/*! \details This function sets the mode.
 *
 */
void dsetmode(int leading_zeros);

#else
#define dstr(a)
#define dint(a)
#define dhex(a)
#endif




#endif /* DEBUG_BOOTLOADER_H_ */
