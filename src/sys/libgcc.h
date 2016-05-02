/*
 * libgcc.h
 *
 *  Created on: Apr 17(void); 2013
 *      Author: tgil
 */

#ifndef LIBGCC_H_
#define LIBGCC_H_

#include <stdint.h>

extern uint32_t __aeabi_uidiv(void);
extern uint32_t __aeabi_uidivmod(void);
extern uint32_t __aeabi_idiv(void);
extern uint32_t __aeabi_idivmod(void);
extern uint32_t __umodsi3(void);
extern uint32_t __modsi3(void);
extern void __div0(void);
extern uint32_t __aeabi_llsr(void);
extern uint32_t __aeabi_lasr(void);
extern uint32_t __aeabi_llsl(void);
extern uint32_t __aeabi_dneg(void);
extern uint32_t __aeabi_drsub(void);
extern uint32_t __aeabi_dsub(void);
extern uint32_t __adddf3(void);
extern uint32_t __aeabi_ui2d(void);
extern uint32_t __aeabi_i2d(void);
extern uint32_t __aeabi_f2d(void);
extern uint32_t __aeabi_ul2d(void);
extern uint32_t __aeabi_l2d(void);
extern uint32_t __aeabi_dmul(void);
extern uint32_t __aeabi_ddiv(void);
extern uint32_t __gedf2(void);
extern uint32_t __ledf2(void);
extern uint32_t __cmpdf2(void);
extern uint32_t __aeabi_cdrcmple(void);
extern uint32_t __aeabi_cdcmpeq(void);
extern uint32_t __aeabi_dcmpeq(void);
extern uint32_t __aeabi_dcmplt(void);
extern uint32_t __aeabi_dcmple(void);
extern uint32_t __aeabi_dcmpge(void);
extern uint32_t __aeabi_dcmpgt(void);
extern uint32_t __aeabi_dcmpun(void);
extern uint32_t __aeabi_d2iz(void);
extern uint32_t __aeabi_d2uiz(void);
extern uint32_t __aeabi_d2f(void);
extern uint32_t __aeabi_fneg(void);
extern uint32_t __aeabi_frsub(void);
extern uint32_t __aeabi_fsub(void);
extern uint32_t __aeabi_fadd(void);
extern uint32_t __addsf3(void);
extern uint32_t __aeabi_ui2f(void);
extern uint32_t __aeabi_i2f(void);
extern uint32_t __aeabi_ul2f(void);
extern uint32_t __aeabi_l2f(void);
extern uint32_t __aeabi_fmul(void);
extern uint32_t __aeabi_fdiv(void);
extern uint32_t __gesf2(void);
extern uint32_t __lesf2(void);
extern uint32_t __cmpsf2(void);
extern uint32_t __aeabi_cfrcmple(void);
extern uint32_t __aeabi_cfcmpeq(void);
extern uint32_t __aeabi_fcmpeq(void);
extern uint32_t __aeabi_fcmplt(void);
extern uint32_t __aeabi_fcmple(void);
extern uint32_t __aeabi_fcmpge(void);
extern uint32_t __aeabi_fcmpgt(void);
extern uint32_t __aeabi_fcmpun(void);
extern uint32_t __aeabi_f2iz(void);
extern uint32_t __aeabi_f2uiz(void);
extern uint32_t __aeabi_f2ulz(void);
extern uint32_t __clzsi2(void);
extern uint32_t __clzdi2(void);
extern uint32_t __muldi3(void);
extern uint32_t __negdi2(void);
extern uint32_t __cmpdi2(void);
extern uint32_t __ucmpdi2(void);
extern uint32_t __absvsi2(void);
extern uint32_t __absvdi2(void);
extern uint32_t __addvsi3(void);
extern uint32_t __addvdi3(void);
extern uint32_t __subvsi3(void);
extern uint32_t __subvdi3(void);
extern uint32_t __mulvsi3(void);
extern uint32_t __mulvdi3(void);
extern uint32_t __negvsi2(void);
extern uint32_t __negvdi2(void);
extern uint32_t __ffsdi2(void);
extern uint32_t __popcountsi2(void);
extern uint32_t __popcountdi2(void);
extern uint32_t __paritysi2(void);
extern uint32_t __paritydi2(void);
extern uint32_t __powisf2(void);
extern uint32_t __powidf2(void);
extern float _Complex __mulsc3(float __a, float __b, float __c, float __d);
extern double _Complex __muldc3(double __a, double __b, double __c, double __d);
extern float _Complex __divsc3(float __a, float __b, float __c, float __d);
extern double _Complex __divdc3(double __a, double __b, double __c, double __d);
extern uint32_t __bswapsi2(void);
extern uint32_t __bswapdi2(void);
extern uint32_t __fixsfdi(void);
extern uint32_t __fixdfdi(void);
extern uint32_t __fixunssfdi(void);
extern uint32_t __fixunsdfdi(void);
extern uint32_t __divdi3(void);
extern uint32_t __moddi3(void);
extern uint32_t __udivdi3(void);
extern uint32_t __umoddi3(void);
extern uint32_t __aeabi_f2lz(void);
extern uint32_t __aeabi_dadd(void);
extern uint32_t __aeabi_d2lz(void);
extern uint32_t __aeabi_d2ulz(void);
extern uint32_t __gnu_f2h_ieee(void);
extern uint32_t __gnu_h2f_ieee(void);

#endif /* LIBGCC_H_ */
