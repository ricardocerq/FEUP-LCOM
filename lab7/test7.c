#include "test7.h"

int ser_test_conf(unsigned short base_addr) {
    return ser_conf(base_addr);
}

int ser_test_set(unsigned short base_addr, unsigned long bits, unsigned long stop, 
	           long parity, unsigned long rate) { 
    /* To be completed */
}

int ser_test_poll(unsigned short base_addr, unsigned char tx, unsigned long bits, 
                    unsigned long stop, long parity, unsigned long rate, 
                    int stringc, char *strings[]) {
    /* To be completed */
}

int ser_test_int(/* details to be provided */) { 
    /* To be completed */
}

int ser_test_fifo(/* details to be provided */) {
    /* To be completed */
}

int ser_com(unsigned short base_addr, unsigned char tx, unsigned long bits,
                    unsigned long stop, long parity, unsigned long rate,
                    char *strings[], int numstrings)
{
	if(tx == 0)
		return ser_int_receive(base_addr, bits, stop, parity,rate);
	else
	return ser_int_transmit(base_addr, bits, stop, parity, rate, strings, numstrings);
}

