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

#ifndef DEBUG_BOOTLOADER_H_
#define DEBUG_BOOTLOADER_H_

#if defined ___debug || defined __debug
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
int dstr(const char * str);
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

#define DEBUG_BOOTLOADER 1

#define DTRACE_LINE() do { dstr(__FUNCTION__); dstr("():"); dint(__LINE__); dstr("\n"); } while(0)

#define DTRACE_INT_PARAMETER(x,y) do { dstr(x); dint(y); dstr("\n"); } while(0)
#define DTRACE_UINT_PARAMETER(x,y) do { dstr(x); duint(y); dstr("\n"); } while(0)
#define DTRACE_HEX_PARAMETER(x,y) do { dstr(x); dhex(y); dstr("\n"); } while(0)
#define DTRACE_BIN_PARAMETER(x,y) do { dstr(x); dbin(y); dstr("\n"); } while(0)

#else
#define dstr(a)
#define dint(a)
#define dhex(a)
#define DTRACE_LINE()
#define DTRACE_INT_PARAMETER(x,y)
#define DTRACE_UINT_PARAMETER(x,y)
#define DTRACE_HEX_PARAMETER(x,y)
#define DTRACE_BIN_PARAMETER(x,y)

#endif




#endif /* DEBUG_BOOTLOADER_H_ */
