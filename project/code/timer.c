#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>

#include "timer.h"
#include "i8254.h"

#define TEST_INT_FREQ 41 //is a divisor of TIMER_FREQ
#define DEFAULT_FREQ 60
#define INITIAL_HOOK_ID_VALUE 0//value between 0-30, 31 returns negative value

static unsigned long int intrpcnt = 0;
static int hook_id = INITIAL_HOOK_ID_VALUE;

unsigned long get_ticks(void)
{
	return intrpcnt;
}

int timer_set_square(unsigned long timer, unsigned long freq) {
	if(timer > 2 || freq == 0){
		printf("timer_set_square() failed: invalid arguments\n");
		return 1;
	}
	//ctrl_word format : SC1|SC0|RW1|RW0|M2|M1|M0|BCD
	//select counter|read write|mode|bcd
	unsigned long ctrl_word = timer << 6; //2 msb set according to timer
	unsigned char conf;
	if(timer_get_conf(timer, &conf)){//get previous configuration
		printf("timer_set_square(): timer_get_conf() failed\n");
		return 1;
	}
	conf &= 0xF;//preserve 4 least significant bits
	ctrl_word = ctrl_word | TIMER_LSB_MSB | conf; //set bits RW1,RW0,(M1,M0,BCD are kept from previous configuration)
	if(sys_outb(TIMER_CTRL, ctrl_word)!= OK){//write control word
		printf("timer_set_square(): sys_outb() failed\n");
		return 1;
	}
	//TIMER_FREQ/div = freq <=> div = TIMER_FREQ/freq;
	unsigned long div = TIMER_FREQ/freq;
	unsigned long out = div & 0xFF;
	if(sys_outb(TIMER_0+timer, out)!= OK){//write LSB
		printf("timer_set_square(): sys_outb() failed\n");
		return 1;
	}
	out = div >> 8;
	if(sys_outb(TIMER_0+timer,out)!= OK){//write MSB
		printf("timer_set_square(): sys_outb() failed\n");
		return 1;
	}
	//printf("timer_set_square() completed successfuly\n");
	return 0;
}

int timer_subscribe_int(void ) {
	int returnvalue = BIT(hook_id);
	if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id)!= OK){
		printf("timer_subscribe_int(): sys_irqsetpolicy() failed\n");
		return -1;
	}
	if(sys_irqenable(&hook_id)!=OK){
		printf("timer_subscribe_int(): sys_irqenable() failed\n");
		return -1;
	}
	return returnvalue;//return value with bit number  original hook_id set to 1
}

int timer_unsubscribe_int() {
	if(sys_irqdisable(&hook_id) != OK){
		printf("timer_unsubscribe_int(): sys_irqdisable() failed\n");
		return 1;
	}
	if(sys_irqrmpolicy(&hook_id) != OK){
		printf("timer_unsubscribe_int(): sys_irqrmpolicy() failed\n");
		return 1;
	}
	return 0;
}

void timer_int_handler() {
	intrpcnt++;//increment global variable
}

int timer_get_value(unsigned long timer, unsigned long *l)//untested
{
	if(timer > 2)//invalid timer
		return 1;
	unsigned char currentconf = 0;
	if(timer_get_conf(timer, &currentconf))//read current config of timer
		return 1;
	unsigned char readwritemode = SEP_BITS(currentconf, 4, 5); //obtain readwritemode
	//counter-latch command format SC1|SC0|0|0|X|X|X|X
		//							SC: select counter, X should be 0
	unsigned long cl_command = timer << 6;
	unsigned long lsb;
	unsigned long msb;
	if(sys_outb(TIMER_CTRL, cl_command)!=OK)
		return 1;
	switch(readwritemode){
		case 1: //least significant byte only.
			if(sys_inb(TIMER_0 + timer, &lsb)!=OK)
				return 1;
			*l = lsb;
			break;
		case 2: //most significant byte only.
			if(sys_inb(TIMER_0 + timer, &msb)!=OK)
				return 1;
			*l = msb << 8;
			break;

		case 3: //least significant byte first then most significant byte
			if(sys_inb(TIMER_0 + timer, &lsb) != OK)
				return 1;
			if(sys_inb(TIMER_0 + timer, &msb) != OK)

			*l = (msb<<8) + lsb;
			break;
		default:
			return 1;
			break;
	}

	return 0;
}

int timer_get_conf(unsigned long timer, unsigned char *st) {
	if(timer > 2){
		printf("timer_get_conf() failed, invalid timer\n");
		return 1;
	}
	//read-back format : 1|1|~count|~status|cnt2|cnt1|cnt0|0
	unsigned long rb_command;
	rb_command = TIMER_RB_CMD | TIMER_RB_COUNT_ |TIMER_RB_SEL(timer); 	//set 2 msb and selector for the timer, everything else  stays at 0
	if(sys_outb(TIMER_CTRL, rb_command)!=OK){//send read-back command to control register
		printf("timer_get_conf(): sys_outb() failed\n");
		return 1;
	}
	unsigned long in;
	if(sys_inb(TIMER_0 + timer, &in)!= OK){	  //read status of timer
		printf("timer_get_conf(): sys_inb() failed\n");
		return 1;
	}
	*st = (unsigned char) in;//assign read value to return pointer
	return 0;
}

int timer_display_conf(unsigned char conf) {
	//OUTPUT|NULLCOUNT|RW1|RW0|M2|M1|M0|BCD
	printf("OUTPUT: %d\n", SEP_BIT(conf,7));
	printf("NULLCOUNT: %d\n", SEP_BIT(conf, 6));
	unsigned char readwrite = SEP_BITS(conf, 4, 5);
	printf("READWRITE: %d ", readwrite);
	switch(readwrite){
	case 1:
		printf("LSB ONLY\n");
		break;
	case 2:
		printf("MSB ONLY\n");
		break;
	case 3:
		printf("LSB AND MSB\n");
		break;
	default:
		printf("timer_display_conf() failed\n");
		return 1;
		break;
	}
	unsigned char mode = SEP_BITS(conf,1,3);
	printf("MODE: %d ", mode);
	switch(mode){
	case 0:
		printf("INTERRUPT ON TERMINAL COUNT\n");
		break;
	case 1:
		printf("HARDWARE RETRIGGERABLE ONE-SHOT\n");
		break;
	case 2:
		printf("RATE GENERATOR\n");
		break;
	case 3:
		printf("SQUARE WAVE\n");
		break;
	case 4:
		printf("SOFTWARE TRIGGERABLE STROBE\n");
		break;
	case 5:
		printf("HARDWARE TRIGGERABLE STROBE\n");
		break;
	default:
		printf("timer_display_conf() failed, invalid mode: %d\n", mode);
		return 1;
	}
	printf("BCD: %d", BIT(0) & conf);
	if(BIT(0) & conf)
		printf(" COUNTING IN BCD\n");
	else printf(" COUNTING IN BINARY\n");
	return 0;
}

int timer_test_square(unsigned long freq) {
	if(freq == 0){
		printf("timer_test_square() failed: invalid frequency\n");
		return 1;
	}
	if(timer_set_square(0, freq)){
		printf("timer_test_square(): timer_set_square() failed\n");
		return 1;
	}
	return 0;
}


int timer_test_int(unsigned long time) {
	if(time == 0){
		printf("timer_test_int() failed: invalid time \n");
		return 1;
	}
	if(timer_set_square(0, TEST_INT_FREQ)){//ensure defined frequency
		printf("timer_test_int(): timer_set_square() failed \n");
		return 1;
	}
	int irq_set;//bitmask
	if((irq_set = timer_subscribe_int()) < 0){//subscribe timer 0 interrupts
		printf("timer_test_int(): timer_subscribe_int() failed \n");
		return 1;
	}
	int ipc_status;
	message msg;
	int r;
	while( intrpcnt <= time*TEST_INT_FREQ) { //while time seconds have not yet passed
		/* Get a request message. */
	    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
	        printf("driver_receive failed with: %d", r);
	        continue;
	    }
	    if (is_ipc_notify(ipc_status)) { /* received notification */
	        switch (_ENDPOINT_P(msg.m_source)) {
	            case HARDWARE: /* hardware interrupt notification */
	                if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
	                    if(intrpcnt % TEST_INT_FREQ == 0)printf("%d\n", intrpcnt/TEST_INT_FREQ);//every second, print current second
	                    timer_int_handler();
	                }
	                break;
	            default:
	                break; /* no other notifications expected: do nothing */
	        }
	    } else {/* received a standard message, not a notification */
	        /* no standard messages expected: do nothing */
	    }
	 }
	intrpcnt = 0;
	if(timer_unsubscribe_int()){//unsubscribe interrupts
		printf("timer_test_int(): timer_unsubscribe_int() failed\n");
		return 1;
	}
	if(timer_set_square(0, DEFAULT_FREQ)){ // reset default frequency
		printf("timer_test_int(): timer_set_square() failed\n");
		return 1;
	}
	printf("Done\n");
	return 0;
}

int timer_test_config(unsigned long timer) {
	unsigned char st;
	if(timer_get_conf(timer, &st)){//read timer configuration
		printf("timer_test_config(): timer_get_conf() failed\n");
		return 1;
	}
	if(timer_display_conf(st)){//display timer configuration
		printf("timer_test_config(): timer_display_conf() failed\n");
		return 1;
	}
	return 0;
}

int timer_test_sound(unsigned long freq)//untested
{
	if(timer_set_square(2, freq))
		return 1;
	unsigned long in;
	if(sys_inb(SPEAKER_CTRL,&in)!= OK)
		return 1;
	unsigned long saved = in;
	in |=0x3;
	if(sys_outb(SPEAKER_CTRL,in)!=OK)
		return 1;
	if(timer_test_int(5))
		return 1;
	if(sys_outb(SPEAKER_CTRL,saved)!= OK)
		return 1;
	return 0;
}
