
#include <minix/drivers.h>
#include "test4.h"
#include "mouse.h"
#include "vector.h"


static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);

int main(int argc, char **argv)
{
	sef_startup();
	sys_enable_iop(SELF);
	if ( argc == 1 ) {
	      print_usage(argv);
	      return 0;
	  } else {
	      if(proc_args(argc, argv))
	    	  printf("Test Unsuccessful\n");
	  }
	  return 0;
	return 0;
}

static void print_usage(char *argv[]) {
  printf("Usage: one of the following:\n"
	 "\t service run %s -args \"test_packet <count>\" \n"
	 "\t service run %s -args \"test_async <idle_time>\" \n"
	 "\t service run %s -args \"test_config\" \n"
     "\t service run %s -args \"test_gesture <length> <tol>\" \n",
	 argv[0],argv[0],argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned long ulongvalue;
	long longvalue;
	unsigned short count,idle_time, tolerance;
	short length;
	if (strncmp(argv[1], "test_packet", strlen("test_packet")) == 0) {
		if( argc != 3 ) {
			printf("mouse: wrong no of arguments for test of mouse_test_packet \n");
			return 1;
		}
		if( (ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		count = (unsigned short) ulongvalue;
		printf("mouse_test_scan:: test_packet(%d)\n",count);
		return test_packet(count);
	}
	if (strncmp(argv[1], "test_async", strlen("test_async")) == 0) {
		if( argc != 3 ) {
			printf("mouse: wrong no of arguments for test of test_async\n");
			return 1;
		}
		if( (ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		idle_time = (unsigned short) ulongvalue;
		printf("mouse:: test_async(%d)\n", idle_time);
		return test_async(idle_time);
	}
	if (strncmp(argv[1], "test_config", strlen("test_config")) == 0) {
		if( argc != 2 ) {
			printf("mouse: wrong no of arguments for test of test_config \n");
			return 1;
		}
		printf("mouse:: test_config()\n");
		return test_config();
	}
	if (strncmp(argv[1], "test_gesture", strlen("test_gesture")) == 0) {
			if( argc != 4 ) {
				printf("mouse: wrong no of arguments for test of test_gesture \n");
				return 1;
			}
			if( (longvalue = parse_long(argv[2], 10)) == LONG_MAX )
				return 1;
			length = (short) longvalue;
			if( (ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
				return 1;
			tolerance = (unsigned short) ulongvalue;
			printf("mouse:: test_gesture(%d,%d)\n", length, tolerance);
			return test_gesture(length,tolerance);
		}
	printf("mouse:: invalid function\n\n");
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
	  printf("mouse: parse_ulong: no digits were found in %s \n", str);
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
	  printf("mouse: parse_long: no digits were found in %s \n", str);
	  return LONG_MAX;
  }

  /* Successful conversion */
  return val;
}
