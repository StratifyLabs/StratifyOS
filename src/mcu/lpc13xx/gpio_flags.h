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

#ifndef GPIO_FLAGS_H_
#define GPIO_FLAGS_H_

static inline void gpio_init() MCU_ALWAYS_INLINE;
static inline void gpio_set(int p, int mask) MCU_ALWAYS_INLINE;
static inline void gpio_clr(int p, int mask) MCU_ALWAYS_INLINE;
static inline int gpio_tst(int p, int mask) MCU_ALWAYS_INLINE;
static inline void gpio_toggle(int p, int mask) MCU_ALWAYS_INLINE;
static inline void gpio_set_dir(int p, int mask) MCU_ALWAYS_INLINE;
static inline void gpio_clr_dir(int p, int mask) MCU_ALWAYS_INLINE;
static inline void gpio_toggle_dir(int p, int mask) MCU_ALWAYS_INLINE;
static inline int gpio_tst_dir(int p, int mask) MCU_ALWAYS_INLINE;
static inline void gpio_wr(int p, int value) MCU_ALWAYS_INLINE;
static inline int gpio_rd(int p) MCU_ALWAYS_INLINE;

void gpio_init(){
	LPC_SYSCON->SYSAHBCLKCTRL |= SYSAHBCLKCTRL_GPIO;
}

void gpio_set(int p, int mask){
	switch(p){
	case 0:
		LPC_GPIO0->MASKED_ACCESS[mask] = (mask);
		break;
	case 1:
		LPC_GPIO1->MASKED_ACCESS[mask] = (mask);
		break;
	case 2:
		LPC_GPIO2->MASKED_ACCESS[mask] = (mask);
		break;
	case 3:
		LPC_GPIO3->MASKED_ACCESS[mask] = (mask);
		break;
	}
}

void gpio_clr(int p, int mask){
	switch(p){
	case 0:
		LPC_GPIO0->MASKED_ACCESS[mask] = 0;
		break;
	case 1:
		LPC_GPIO1->MASKED_ACCESS[mask] = 0;
		break;
	case 2:
		LPC_GPIO2->MASKED_ACCESS[mask] = 0;
		break;
	case 3:
		LPC_GPIO3->MASKED_ACCESS[mask] = 0;
		break;
	}
}

int gpio_tst(int p, int mask){
	switch(p){
	case 0:
		return LPC_GPIO0->MASKED_ACCESS[mask];
		break;
	case 1:
		return LPC_GPIO1->MASKED_ACCESS[mask];
		break;
	case 2:
		return LPC_GPIO2->MASKED_ACCESS[mask];
		break;
	case 3:
		return LPC_GPIO3->MASKED_ACCESS[mask];
		break;
	}
	return 0;
}

void gpio_toggle(int p, int mask){
	switch(p){
	case 0:
		LPC_GPIO0->MASKED_ACCESS[mask] ^= (mask);
		break;
	case 1:
		LPC_GPIO1->MASKED_ACCESS[mask] ^= (mask);
		break;
	case 2:
		LPC_GPIO2->MASKED_ACCESS[mask] ^= (mask);
		break;
	case 3:
		LPC_GPIO3->MASKED_ACCESS[mask] ^= (mask);
		break;
	}
}

void gpio_set_dir(int p, int mask){
	switch(p){
	case 0:
		LPC_GPIO0->DIR |= (mask);
		break;
	case 1:
		LPC_GPIO1->DIR |= (mask);
		break;
	case 2:
		LPC_GPIO2->DIR |= (mask);
		break;
	case 3:
		LPC_GPIO3->DIR |= (mask);
		break;
	}
}


void gpio_clr_dir(int p, int mask){
	switch(p){
	case 0:
		LPC_GPIO0->DIR &= ~(mask);
		break;
	case 1:
		LPC_GPIO1->DIR &= ~(mask);
		break;
	case 2:
		LPC_GPIO2->DIR &= ~(mask);
		break;
	case 3:
		LPC_GPIO3->DIR &= ~(mask);
		break;
	}
}

void gpio_toggle_dir(int p, int mask){
	switch(p){
	case 0:
		LPC_GPIO0->DIR ^= (mask);
		break;
	case 1:
		LPC_GPIO1->DIR ^= (mask);
		break;
	case 2:
		LPC_GPIO2->DIR ^= (mask);
		break;
	case 3:
		LPC_GPIO3->DIR ^= (mask);
		break;
	}
}


int gpio_tst_dir(int p, int mask){
	switch(p){
	case 0:
		return LPC_GPIO0->DIR & (mask);
	case 1:
		return LPC_GPIO1->DIR & (mask);
	case 2:
		return LPC_GPIO2->DIR & (mask);
	case 3:
		return LPC_GPIO3->DIR & (mask);
	}
}

void gpio_wr(int p, int value){
	switch(p){
	case 0:
		LPC_GPIO0->DATA = (value);
		break;
	case 1:
		LPC_GPIO1->DATA = (value);
		break;
	case 2:
		LPC_GPIO2->DATA = (value);
		break;
	case 3:
		LPC_GPIO3->DATA = (value);
		break;
	}
}

int gpio_rd(int p){
	switch(p){
	case 0:
		return LPC_GPIO0->DATA;
	case 1:
		return LPC_GPIO1->DATA;
	case 2:
		return LPC_GPIO2->DATA;
	case 3:
		return LPC_GPIO3->DATA;
	}
}
/*! \endcond */



#endif /* GPIO_FLAGS_H_ */
