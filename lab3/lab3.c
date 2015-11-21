
#include <minix/drivers.h>
#include "test3.h"
#include "kbd.h"

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
	 "\t service run %s -args \"test_scan <ass>\" \n"
	 "\t service run %s -args \"test_leds <leds>\" \n"
	 "\t service run %s -args \"test_timed_scan <n>\" \n",
	 argv[0],argv[0],argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned short ass,numelems, numsecs;
	unsigned long elem;
	if (strncmp(argv[1], "test_scan", strlen("test_scan")) == 0) {
		  if( argc != 3 ) {
			  printf("kbd: wrong no of arguments for test of kbd_test_scan \n");
			  		  return 1;
		  }
		  if( (elem = parse_ulong(argv[2], 10)) == ULONG_MAX )
		  		  return 1;
		  ass = (unsigned short) elem;
		  printf("kbd_test_scan:: kbd_test_scan(%d)\n",ass);
		  return kbd_test_scan(ass);
		}
	if (strncmp(argv[1], "test_leds", strlen("test_leds")) == 0) {
			  if( argc == 2 ) {
				  printf("kbd: wrong no of arguments for test of kbd_test_leds \n");
				  		  return 1;
			  }
			  numelems = argc - 2;
			  unsigned short* leds = malloc(numelems * sizeof(unsigned short));
			  size_t i = 0;
			  for(; i < numelems; ++i)
			  {
				  if((elem = parse_ulong(argv[2+i], 10)) == ULONG_MAX )
					  return 1;
				  leds[i] = (unsigned short) elem;
			  }
			  printf("kbd:: test_leds( ");
			  int returnvalue = kbd_test_leds(numelems,leds);
			  free(leds);
			  return returnvalue;
		}
	if (strncmp(argv[1], "test_timed_scan", strlen("test_timed_scan")) == 0) {
				  if( argc != 3 ) {
					  printf("kbd: wrong no of arguments for test of test_timed_scan \n");
					  		  return 1;
				  }
				  if( (elem = parse_ulong(argv[2], 10)) == ULONG_MAX )
				  		  return 1;
				  numsecs = (unsigned short) elem;
				  printf("kbd:: test_timed_scan(%d)\n",numsecs);
				  return kbd_test_timed_scan(numsecs);
			}
	printf("kbd:: invalid function\n\n");
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
	  printf("kbd: parse_ulong: no digits were found in %s \n", str);
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
	  printf("kbd: parse_long: no digits were found in %s \n", str);
	  return LONG_MAX;
  }

  /* Successful conversion */
  return val;
}
