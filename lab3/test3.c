#include "test3.h"
#include "kbd.h"


//int kbd_scan_internal(int (*ih_wrapper)(unsigned long *,unsigned long *));

int kbd_test_scan(unsigned short ass) {
	int (*ih)(unsigned long *,unsigned long *) = NULL;
	if(ass == 0)
		ih = scan_on_kbd_int_c;
	else {
		ih = scan_on_kbd_int_ass;
	}
	return kbd_scan_internal(ih);
}


int kbd_test_leds(unsigned short n, unsigned short *leds) {
	size_t i = 0;
	for(; i < n ; ++i)
	{
		printf("%d ", leds[i]);
		if(leds[i] > 2)
		{
			printf("\nInvalid led in array\n");
			return 1;
		}
	}
	printf(")\n");
	return kbd_test_leds_internal(n, leds);
}
int kbd_test_timed_scan(unsigned short n) {
   if(n == 0)
   {
	   printf("\nInvalid time\n");
	   return 1;
   }
   else return  kbd_test_timed_scan_internal(n);
}
