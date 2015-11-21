#include <minix/drivers.h>

#include "timer.h"
#include "i8254.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);

int main(int argc, char **argv)
{
	sef_startup();
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
	 "\t service run %s -args \"test_config <timer>\" \n"
	 "\t service run %s -args \"test_square <freq>\" \n"
	 "\t service run %s -args \"test_int <time>\" \n",
	 argv[0],argv[0],argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned long timer,freq, time;
	if (strncmp(argv[1], "test_config", strlen("test_config")) == 0) {
		  if( argc != 3 ) {
			  printf("timer: wrong no of arguments for test of timer_test_config \n");
			  		  return 1;
		  }
		  if( (timer = parse_ulong(argv[2], 10)) == ULONG_MAX )
		  		  return 1;
		  printf("timer:: timer_test_config(%d)\n",timer);
		  return timer_test_config(timer);
		}
	if (strncmp(argv[1], "test_square", strlen("test_square")) == 0) {
			  if( argc != 3 ) {
				  printf("timer: wrong no of arguments for test of timer_test_square \n");
				  		  return 1;
			  }
			  if( (freq = parse_ulong(argv[2], 10)) == ULONG_MAX )
			  		  return 1;
			  printf("timer:: timer_test_square(%d)\n",freq);
			  return timer_test_square(freq);
		}
	if (strncmp(argv[1], "test_int", strlen("test_int")) == 0) {
				  if( argc != 3 ) {
					  printf("timer: wrong no of arguments for test of timer_test_int \n");
					  		  return 1;
				  }
				  if( (time = parse_ulong(argv[2], 10)) == ULONG_MAX )
				  		  return 1;
				  printf("timer:: timer_test_int(%d)\n",time);
				  return timer_test_int(time);
			}
	if (strncmp(argv[1], "test_sound", strlen("test_int")) == 0) {
					  if( argc != 3 ) {
						  printf("timer: wrong no of arguments for test of timer_test_int \n");
						  		  return 1;
					  }
					  if( (freq = parse_ulong(argv[2], 10)) == ULONG_MAX )
					  		  return 1;
					  printf("timer:: timer_test_sound(%d)\n",freq);
					  return timer_test_sound(freq);
				}
	printf("timer:: invalid function\n\n");
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
	  printf("timer: parse_ulong: no digits were found in %s \n", str);
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
	  printf("timer: parse_long: no digits were found in %s \n", str);
	  return LONG_MAX;
  }

  /* Successful conversion */
  return val;
}
