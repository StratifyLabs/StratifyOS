
#include "sos/symbols.h"
#include "sos/symbols/table.h"


void __cxa_pure_virtual(){}

void __div0(){
	while(1){}
}

u32 symbols_total(){
	int i;
	i = 0;
	while(symbols_table[i] != 1 ){
		i++;
	}
	return i;
}



