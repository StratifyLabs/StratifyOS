/*
 * debug.h
 *
 *  Created on: Sep 28, 2013
 *      Author: tgil
 */

#ifndef DEBUG_BOOTLOADER_H_
#define DEBUG_BOOTLOADER_H_

#ifdef ___debug
#include "mcu/debug.h"


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
