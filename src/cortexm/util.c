
#include "cortexm/util.h"

char htoc(int nibble) {
  if (nibble >= 0 && nibble < 10) {
    return (char)nibble + '0';
  } else {
    return (char)nibble + 'A' - 10;
  }
}

void htoa(char *dest, int num) {
  int i;
  for (i = 0; i < 8; i++) {
    char nibble;
    nibble = num & 0xF;
    num >>= 4;
    dest[7 - i] = htoc(nibble);
  }
  dest[8] = 0;
}
