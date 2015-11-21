#include "test7.h"
#include "ser_port.h"
#include "char_queue.h"
#include <minix/drivers.h>

//int main(int argc, char **argv) {
//	sef_startup();
////	ser_conf(1);
////	printf("\n\n");
////	ser_set(1, 8, 1, 3, 9600);
////	ser_conf(1);
//	ser_int_receive(1, 8, 1, 3,9600);
//	return 0;
//}
//
//#include <minix/drivers.h>
//#include "test6.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);


int main(int argc, char **argv)
{
	sef_startup();
	//sys_enable_iop(SELF);
	printf("ok\n");
	if ( argc == 1 ) {
		print_usage(argv);
		return 0;
	} else {
		if(proc_args(argc, argv))
			printf("Test Unsuccessful\n");
	}
	return 0;
}


static void print_usage(char *argv[]) {
  printf("Usage: one of the following:\n"
	 "\t service run %s -args \"conf 1|2\" \n"
	 "\t service run %s -args \"set 1|2 <bits> <stop> <parity> <rate>\" \n"
	 "\t service run %s -args \"com 1|2 tx|rx <bits> <stop> <parity> <rate> <string>*\" \n",
	 argv[0],argv[0],argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned long ulongvalue;
	long longvalue;
	unsigned short base_addr;
	unsigned long bits;
	unsigned long stop;
	long parity;
	unsigned long rate;
	unsigned short tx;
	char** strings = 0;
	int numstrings = 0;
	if (strncmp(argv[1], "conf", strlen("conf")) == 0) {
		if( argc != 3 ) {
			printf("uart: wrong no of arguments for test of ser_conf()\n");
			return 1;
		}
		if( (ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		base_addr = (unsigned short) ulongvalue;
		printf("uart::ser_conf(%d)\n", base_addr);
		return ser_conf(base_addr);
	}
	if (strncmp(argv[1], "set", strlen("set")) == 0) {
		if( argc != 7 ) {
			printf("uart: wrong no of arguments for test of ser_set() \n");
			return 1;
		}
		if( (ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		base_addr = (unsigned short) ulongvalue;
		if( (ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		bits = ulongvalue;
		if( (ulongvalue = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		stop = ulongvalue;
		if( (ulongvalue = parse_ulong(argv[6], 10)) == ULONG_MAX )
			return 1;
		rate = ulongvalue;
		if (strncmp(argv[5], "odd", strlen("odd")) == 0)
			parity = 1;
		else if (strncmp(argv[5], "even", strlen("even")) == 0)
			parity = 3;
		else if (strncmp(argv[5], "1", strlen("1")) == 0)
			parity = 5;
		else if (strncmp(argv[5], "0", strlen("0")) == 0)
			parity = 7;
		else
			parity = 0;

		printf("uart:: ser_set(%d,%d,%d,%d,%d)\n",base_addr,bits, stop, parity,rate);
		return ser_set(base_addr,bits, stop, parity,rate);
	}
	if (strncmp(argv[1], "com", strlen("com")) == 0) {
		if( argc < 8 ) {
			printf("uart: wrong no of arguments for test of ser_com \n");
			return 1;
		}
		if( (ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		base_addr = (unsigned short) ulongvalue;
		if (strncmp(argv[3], "tx", strlen("tx")) == 0)
					tx = 1;
		else tx = 0;
		if( (ulongvalue = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		bits = ulongvalue;
		if( (ulongvalue = parse_ulong(argv[5], 10)) == ULONG_MAX )
			return 1;
		stop = ulongvalue;
		if( (ulongvalue = parse_ulong(argv[7], 10)) == ULONG_MAX )
			return 1;
		rate = ulongvalue;
		if (strncmp(argv[6], "odd", strlen("odd")) == 0)
			parity = 1;
		else if (strncmp(argv[6], "even", strlen("even")) == 0)
			parity = 3;
		else if (strncmp(argv[6], "1", strlen("1")) == 0)
			parity = 5;
		else if (strncmp(argv[6], "0", strlen("0")) == 0)
			parity = 7;
		else
			parity = 0;
		if(argc > 8)
			{
				strings = (char**)argv[8];
				numstrings = argc - 8;
			}

		printf("uart:: ser_com(%d,%d,%d,%d,%d,%d,%d,%d)\n", base_addr,tx, bits, stop, parity,rate, strings, numstrings);
		return ser_com(base_addr,tx, bits, stop, parity,rate, strings, numstrings);
	}
	printf("uart:: invalid function\n\n");
	return 1;
}
static unsigned long parse_ulong(char *str, int base) {
  char *endptr;
  unsigned long val;

  val = strtoul(str, &endptr, base);

  if ((errno == ERANGE && val == ULONG_MAX )
	  || (errno != 0 && val == 0)) {
	  perror("strtol");
	  return ULONG_MAX;
  }

  if (endptr == str) {
	  printf("graphic_mode: parse_ulong: no digits were found in %s \n", str);
	  return ULONG_MAX;
  }

  /* Successful conversion */
  return val;
}

static long parse_long(char *str, int base) {
  char *endptr;
  unsigned long val;

  val = strtol(str, &endptr, base);

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
	  || (errno != 0 && val == 0)) {
	  perror("strtol");
	  return LONG_MAX;
  }

  if (endptr == str) {
	  printf("graphic_mode: parse_long: no digits were found in %s \n", str);
	  return LONG_MAX;
  }

  /* Successful conversion */
  return val;
}
