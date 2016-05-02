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
 */



#include <stdint.h>
#include "mcu/boot_debug.h"

#ifdef ___debug

#define BUF_SIZE 36

static int width;
static int (*writefunc)(const void *, int);

static char Htoc(int nibble);
static char htoc(int nibble);
static int ditoa(char dest[BUF_SIZE], int32_t num, int width);
static int dutoa(char dest[BUF_SIZE], uint32_t num, int base, char upper, int width);
static void dwrite(char * str);

void dsetmode(int leading_zeros){
	width = leading_zeros;
}

void dsetwritefunc(int (*func)(const void *, int)){
	writefunc = func;
}

int dint(int x){
	char buf[BUF_SIZE];
	ditoa(buf, x, width);
	dwrite(buf);
	return 0;
}

int duint(unsigned int x){
	char buf[BUF_SIZE];
	dutoa(buf, x, 10, 1, width);
	dwrite(buf);
	return 0;
}

int dstr(char * str){
	dwrite(str);
	return 0;
}

int dhex(int x){
	char buf[BUF_SIZE];
	dutoa(buf, x, 16, false, width);
	dwrite(buf);
	return 0;
}

int dHex(int x){
	char buf[BUF_SIZE];
	dutoa(buf, x, 16, true, width);
	dwrite(buf);
	return 0;
}

int dbin(int x){
	char buf[BUF_SIZE];
	dutoa(buf, x, 2, false, width);
	dwrite(buf);
	return 0;
}

void dwrite(char * str){
	if ( writefunc != NULL ){
		writefunc(str, strlen(str));
	}
}


char Htoc(int nibble){
	if ( nibble >= 0 && nibble < 10 ){
		return nibble + '0';
	} else {
		return nibble + 'A' - 10;
	}
}

char htoc(int nibble){
	if ( nibble >= 0 && nibble < 10 ){
		return nibble + '0';
	} else {
		return nibble + 'a' - 10;
	}
}

int ditoa(char dest[BUF_SIZE], int32_t num, int width){
	char buf[BUF_SIZE];
	int i, j;
	char started;
	char neg;
	j = 0;
	neg = 0;
	if( num < 0 ){
		neg = 1;
		num *= -1;
	}
	memset(buf, '0', BUF_SIZE);
	do {
		buf[j++] = htoc(num%10);
		num /= 10;
	} while( (num >  0) && (j < BUF_SIZE));
	started = 0;
	j = 0;
	if( neg != 0 ){
		dest[j++] = '-';
	}
	for(i=BUF_SIZE-1; i >= 0; i--){
		if( (buf[i] != '0') || (started != 0) || (i<width)){
			started = 1;
			dest[j++] = buf[i];
		}
	}

	if( (j == 0) || ((j == 1) && neg) ){
		dest[j++] = '0';
	}

	dest[j] = 0;
	return j;
}

int dutoa(char dest[BUF_SIZE], uint32_t num, int base, char upper, int width){
	char buf[BUF_SIZE];
	int i, j;
	char started;
	char c;
	j = 0;
	memset(buf, '0', BUF_SIZE);
	while( (num > 0) && (j < BUF_SIZE)){
		if( upper ){
			c = Htoc(num%base);
		} else {
			c = htoc(num%base);
		}
		buf[j++] = c;
		num /= base;
	};
	started = 0;
	j = 0;
	for(i=BUF_SIZE-1; i >= 0; i--){
		if( (buf[i] != '0') || (started != 0) || (i<width)){
			started = 1;
			dest[j++] = buf[i];
		}
	}
	if( j == 0 ){
		dest[j++] = '0';
	}
	dest[j] = 0;
	return j;
}

#endif
