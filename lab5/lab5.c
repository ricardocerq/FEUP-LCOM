
#include <minix/drivers.h>
#include "test5.h"
#include "pixmap.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);


int main(int argc, char **argv)
{
	sef_startup();
	lm_init();
	sys_enable_iop(SELF);
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
	 "\t service run %s -args \"test_init <mode> <delay>\" \n"
	 "\t service run %s -args \"test_square <x> <y> <size> <color>\" \n"
	 "\t service run %s -args \"test_line <xi> <yi> <xf> <yf> <color>\" \n"
     "\t service run %s -args \"test_xpm <xi> <yi> <xpm>\" \n"
     "\t service run %s -args \"test_move <xi> <yi> <xpm> <hor> <delta> <time>\" \n"
     "\t service run %s -args \"test_controller\" \n"
	 "\t service run %s -args \"test_circle <x> <y> <radius> <color>\" \n",
	 argv[0],argv[0],argv[0], argv[0], argv[0], argv[0],argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned long ulongvalue;
	long longvalue;
	unsigned short mode, delay, xi, yi, size,xf, yf, hor, time;
	short delta;
	unsigned long color;
	char* xpm;
	if (strncmp(argv[1], "test_init", strlen("test_init")) == 0) {
		if( argc != 4 ) {
			printf("graphic_mode: wrong no of arguments for test of test_init \n");
			return 1;
		}
		if( (ulongvalue = parse_ulong(argv[2], 16)) == ULONG_MAX )
			return 1;
		mode = (unsigned short) ulongvalue;
		if( (ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		delay = (unsigned short) ulongvalue;
		printf("graphic_mode::test_init(0x%x,%d)\n",mode, delay);
		if(test_init(mode, delay)==NULL)
			return 1;
		else return 0;
	}
	if (strncmp(argv[1], "test_square", strlen("test_square")) == 0) {
		if( argc != 6 ) {
			printf("graphic_mode: wrong no of arguments for test of test_square \n");
			return 1;
		}
		if((ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		xi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		yi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		size = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[5], 16)) == ULONG_MAX )
			return 1;
		color = ulongvalue;
		printf("graphic_mode:: test_square(%d,%d,%d, %x)", xi, yi, size, color);
		return test_square(xi, yi, size, color);
	}
	if (strncmp(argv[1], "test_line", strlen("test_line")) == 0) {
		if( argc != 7 ) {
			printf("graphic_mode: wrong no of arguments for test of test_line \n");
			return 1;
		}
		if((ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		xi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		yi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		xf = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[5], 10)) == ULONG_MAX )
			return 1;
		yf = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[6], 16)) == ULONG_MAX )
			return 1;
		color = ulongvalue;

		printf("graphic_mode:: test_line(%d,%d,%d,%d,%x)\n",xi,yi,xf,yf,color);
		return test_line(xi,yi,xf,yf,color) ;
	}
	if (strncmp(argv[1], "test_xpm", strlen("test_xpm")) == 0) {
		if( argc != 5 ) {
			printf("graphic_mode: wrong no of arguments for test of test_xpm \n");
			return 1;
		}
		if((ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		xi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		yi = (unsigned short) ulongvalue;
		xpm = argv[4];
		char ** xpmptr;
		if (strncmp(xpm, "pic1", strlen("pic1")) == 0)
			xpmptr = pic1;
		if (strncmp(xpm, "pic2", strlen("pic2")) == 0)
			xpmptr = pic2;
		if (strncmp(xpm, "cross", strlen("cross")) == 0)
			xpmptr = cross;
		if (strncmp(xpm, "pic3", strlen("pic3")) == 0)
			xpmptr = pic3;
		if (strncmp(xpm, "penguin", strlen("penguin")) == 0)
			xpmptr = penguin;
		printf("graphic_mode:: test_xpm(%d,%d, %s)\n", xi, yi,xpm);
		return test_xpm(xi, yi, xpmptr);
	}
	if (strncmp(argv[1], "test_move", strlen("test_move")) == 0) {
		if( argc != 8 ) {
			printf("graphic_mode: wrong no of arguments for test of test_move \n");
			return 1;
		}
		if((ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		xi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		yi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[5], 10)) == ULONG_MAX )
			return 1;
		hor = (unsigned short) ulongvalue;

		if((longvalue = parse_long(argv[6], 10)) == LONG_MAX )
			return 1;
		delta = (short) longvalue;

		if((ulongvalue = parse_ulong(argv[7], 10)) == ULONG_MAX )
			return 1;
		time = (unsigned short) ulongvalue;

		xpm = argv[4];
		char ** xpmptr;
		if (strncmp(xpm, "pic1", strlen("pic1")) == 0)
			xpmptr = pic1;
		if (strncmp(xpm, "pic2", strlen("pic2")) == 0)
			xpmptr = pic2;
		if (strncmp(xpm, "cross", strlen("cross")) == 0)
			xpmptr = cross;
		if (strncmp(xpm, "pic3", strlen("pic3")) == 0)
			xpmptr = pic3;
		if (strncmp(xpm, "penguin", strlen("penguin")) == 0)
			xpmptr = penguin;
		printf("graphic_mode:: test_move(%d,%d, %s, %d, %d, %d)\n", xi, yi,xpm, hor, delta,time);
		return test_move(xi, yi,xpmptr, hor, delta,time);
	}
	if (strncmp(argv[1], "test_controller", strlen("test_controller")) == 0) {
		if( argc != 2 ) {
			printf("graphic_mode: wrong no of arguments for test of test_controller() \n");
			return 1;
		}
		return test_controller();
	}
	if (strncmp(argv[1], "test_circle", strlen("test_circle")) == 0) {
		if( argc != 6 ) {
			printf("graphic_mode: wrong no of arguments for test of test_circle \n");
			return 1;
		}
		if((ulongvalue = parse_ulong(argv[2], 10)) == ULONG_MAX )
			return 1;
		xi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[3], 10)) == ULONG_MAX )
			return 1;
		yi = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[4], 10)) == ULONG_MAX )
			return 1;
		size = (unsigned short) ulongvalue;

		if((ulongvalue = parse_ulong(argv[5], 16)) == ULONG_MAX )
			return 1;
		color = ulongvalue;
		printf("graphic_mode:: test_circle(%d,%d,%d, %x)", xi, yi, size, color);
		return test_circle(xi, yi, size, color);
	}
	printf("graphic_mode:: invalid function\n\n");
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
