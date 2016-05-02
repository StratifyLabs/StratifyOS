
#include "stratify/symbols.h"
#include "stratify/symbols_table.h"

char __aeabi_unwind_cpp_pr0[0];

void __cxa_pure_virtual(void){}

void __div0(void){
	while(1){}
}

u32 symbols_total(void){
	int i;
	i = 0;
	while(symbols_table[i] != 1 ){
		i++;
	}
	return i;
}



