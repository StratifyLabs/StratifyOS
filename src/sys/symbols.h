// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include <sdk/types.h>
#include <stdint.h>

extern u32 const symbols_table[] MCU_WEAK;

static inline u32 symbols_total() {
  int i;
  i = 0;
  while (symbols_table[i] != 1) {
    i++;
  }
  return i;
}

#endif /* SYMBOLS_H_ */
