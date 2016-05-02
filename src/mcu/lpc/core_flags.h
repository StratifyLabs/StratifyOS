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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef CORE_FLAGS_H_
#define CORE_FLAGS_H_



#define OSCRANGE 4
#define OSCEN 5
#define OSCSTAT 6
#define PLLE0 0
#define PLLC0 1
#define PLLE0_STAT 24
#define PLLC0_STAT 25
#define PLOCK0 26



#define PLLE1 0
#define PLLC1 1
#define PLLE1_STAT 8
#define PLLC1_STAT 9
#define PLOCK1 10

#define IRC_OSC 0
#define MAIN_OSC 1


#define PM1 (1<<1)
#define PM0 (1<<0)
#define PM_MASK (0x03)



#define PLLCFG_MSEL(x) (((x)-1) << 0) // PLL Multiplier
#define PLLCFG_PSEL(x) ((x) << 5) // PLL Divider


#endif /* CORE_FLAGS_H_ */
