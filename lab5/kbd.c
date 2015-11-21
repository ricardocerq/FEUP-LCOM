#include "kbd.h"
#include "timer.h"
#include "i8042.h"

#define DELAY_US 20000
#define TEST_INT_FREQ 41 //is a divisor of TIMER_FREQ
#define DEFAULT_FREQ 60

static int kbd_hook_id = ORIGINAL_KBD_HOOK_ID;
static int numseconds = 0;
static unsigned short* ledsptr = NULL;
static unsigned long scancode = 0;
static char firstbyte = 0;

char is_first_byte(void)
{
	if(scancode == TWO_BYTE)
		return 1;
	else return 0;
}
int kbd_int_handler(void)
{
	unsigned long value;
	if(kbd_int_handler_ass(&value))
	{
		printf("kbd int handler failed\n");
		return 1;
	}
	if(is_first_byte())
		scancode = (scancode << 8) + value;
	else scancode = value;
	return 0;
}
unsigned long get_scancode(void)
{
	return scancode;
}

int kbd_subscribe_int(int block_kbd)
{
	int returnvalue = BIT(kbd_hook_id);
	int irqpolicy = IRQ_EXCLUSIVE;
	if(!block_kbd)
		irqpolicy |= IRQ_REENABLE;//dont use reenable for test_leds
	if(sys_irqsetpolicy(KBD_IRQ, irqpolicy, &kbd_hook_id)!= OK){
		printf("kbd_subscribe_int(): sys_irqsetpolicy() failed\n");
		return -1;
	}

	if(sys_irqenable(&kbd_hook_id)!=OK){
		printf("kbd_subscribe_int(): sys_irqenable() failed\n");
		return -1;
	}
	return returnvalue;//return value with bit number (original hook_id) set to 1
}

int kbd_unsubscribe_int(void) {
	if(sys_irqdisable(&kbd_hook_id) != OK){
		printf("kbd_unsubscribe_int(): sys_irqdisable() failed\n");
		return 1;
	}
	if(sys_irqrmpolicy(&kbd_hook_id) != OK){
		printf("kbd_unsubscribe_int(): sys_irqrmpolicy() failed\n");
		return 1;
	}
	return 0;
}

void print_scan_code(unsigned long scancode)
{
	if((scancode & BREAK_CODE_BIT) == 0)
		printf("Makecode : 0x%04x\n", scancode);
	else printf("Breakcode: 0x%04x\n",scancode);
}

int kbd_int_handler_c(unsigned long *st)
{
	if(read_kbd_value(st))
		return 1;
	return 0;
}
//calls a function returning a boolean value with 2 arguments,
char call_condition(char (*condition1)(unsigned long,unsigned long),unsigned long* var1, unsigned long* var2)
{
	if(var1 == NULL && var2 == NULL)//if var pointer is NULL pass 0 instead
		return condition1(0,0);//neither variable exist
	if(var1 == NULL)
		return condition1(0,*var2);
	if(var2 == NULL)
		return condition1(*var1, 0);
	return condition1(*var1, *var2);//both variables exist
}

char condition_merger(char (*condition1)(unsigned long,unsigned long), char (*condition2)(unsigned long,unsigned long), unsigned long* var1, unsigned long* var2)//receives 1 or conditions and 1 or more variables
{
	if(condition1 == NULL)
		if(condition2 == NULL)
		{
			return 0; //if neither condition exists, return false
		} else return call_condition(condition2, var1, var2);//only condition2 exists
	else
		if(condition2 == NULL)
			return call_condition(condition1,var1,var2);//only condition1 exists
		else return call_condition(condition1,var1,var2) && call_condition(condition2, var1, var2); //both conditions exist, call both
}

int generic_interrupt_loop(unsigned long* var_timer, unsigned long* var_kbd, int (*on_timer_int)(unsigned long *,unsigned long *), int (*on_kbd_int)(unsigned long *,unsigned long *), char block_kbd, char (*timer_condition)(unsigned long,unsigned long), char (*kbd_condition)(unsigned long,unsigned long))
{
	if(on_timer_int == NULL && on_kbd_int == NULL)
		return 1;
	int timer_irq_set = 0;
	int kbd_irq_set = 0;
	char failure = 0;//set to 1 if something fails
	if(on_timer_int){//function use needs timer interrupts
		if(timer_set_square(0, TEST_INT_FREQ)){//ensure defined frequency
			printf("generic_interrupt_loop(): timer_set_square() failed \n");
			failure = 1;
		}
		if((timer_irq_set = timer_subscribe_int()) < 0){//subscribe timer 0 interrupts
			printf("generic_interrupt_loop(): timer_subscribe_int() failed \n");
			failure = 1;
		}
	}
	if(on_kbd_int || block_kbd){//function use needs kbd interrupts
		if((kbd_irq_set = kbd_subscribe_int(block_kbd)) < 0){//subscribe kbd interrupts
			printf("generic_interrupt_loop(): kbd_subscribe_int() failed \n");
			failure = 1;
		}
	}
	int ipc_status;
	message msg;
	int r;
	char condition;//to be used in do while condition
	if(!failure){
		do{
			/* Get a request message. */
			if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & timer_irq_set) { /* subscribed timer interrupt */
						if(on_timer_int)//if there is something to do on timer interrupts

							if(on_timer_int(var_timer, var_kbd))
							{
								printf("Action on timer interrupt failed\n");
								failure = 1;
							}
					}
					if (msg.NOTIFY_ARG & kbd_irq_set) { /* subscribed kbd interrupt */
						if(on_kbd_int)//if there is something to do on kbd interrupts
						{
							if(on_kbd_int(var_timer,var_kbd))
							{
								printf("Action on kbd interrupt failed\n");
								failure = 1;
							}
						}
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
			condition = condition_merger(timer_condition, kbd_condition, var_timer, var_kbd);
		} while(condition && !failure);
	}
	if(on_kbd_int || block_kbd)
	{
		if(kbd_unsubscribe_int()){//unsubscribe interrupts
			printf("generic_interrupt_loop(): kbd_unsubscribe_int() failed\n");
			failure = 1;
		}
	}
	if(on_timer_int)
	{
		if(timer_unsubscribe_int()){//unsubscribe interrupts
			printf("generic_interrupt_loop(): timer_unsubscribe_int() failed\n");
			failure = 1;
		}
		if(timer_set_square(0, DEFAULT_FREQ)){ // reset default frequency
			printf("generic_interrupt_loop(): timer_set_square() failed\n");
			failure = 1;
		}
	}
	printf("Done\n");
	return failure;
}
int scan_on_kbd_int_c(unsigned long* ticks,unsigned long *scancode){
	char twobyte= 0;
	unsigned long saved;
	if(*scancode == TWO_BYTE)
		{
			saved = *scancode;
			twobyte = 1;
		}
	if(kbd_int_handler_c(scancode))
	{
		printf("scan_on_kbd_int_c(): kbd_int_handler_c() failed\n");
		return 1;
	}
	if(twobyte)
		*scancode = (saved << 8) + *scancode;//add 2nd byte
	if(ticks != NULL)
		*ticks = 0;
	if(*scancode != TWO_BYTE)	print_scan_code(*scancode);
	return 0;
}

int scan_on_kbd_int_ass(unsigned long* ticks,unsigned long *scancode)
{
	char twobyte= 0;
	unsigned long saved;
	if(*scancode == TWO_BYTE)
		{
			saved = *scancode;
			twobyte = 1;
		}
	if(kbd_int_handler_ass(scancode))
	{
		printf("scan_on_kbd_int_ass(): kbd_int_handler_ass() failed\n");
		return 1;
	}
	if(twobyte)
		*scancode = (saved << 8) + *scancode;
	if(ticks != NULL)
		*ticks = 0;
	if(*scancode != TWO_BYTE)print_scan_code(*scancode);
	return 0;
}

char scan_condition(unsigned long unused, unsigned long scancode)
{
	return scancode != ESC_BREAK;
}

int kbd_scan_internal(int (*ih_wrapper)(unsigned long *,unsigned long *))
{
	if(ih_wrapper == NULL)
		return 1;
	unsigned long scancode;
	if(generic_interrupt_loop(NULL, &scancode, NULL, ih_wrapper, 0, NULL, scan_condition))
		return 1;
	return 0;
}

int unchecked_read_kbd_value(unsigned long* data)
{
	if(sys_inb(OUT_BUF, data)!= OK)
		return 1;
	return 0;
}
//reads a value from keyboard
int read_kbd_value(unsigned long* data)
{
	int attempts = 0;
	unsigned long stat;
	while( 1 ) {
		if(attempts >= MAX_ATTEMPTS)
			return 1;
		if(sys_inb(STAT_REG, &stat)!= OK)
			return 1;
		if( stat & OUT_BUF_STATUS ) {
			if(sys_inb(OUT_BUF, data)!= OK)
				return 1;
			if ( (stat &(PARITY_ERROR | TIMEOUT)) == 0 )//no error
				{
					return 0;
				}
		}
		attempts++;//not ready
	WAIT_MS(WAIT_TIME);
	}
}
//writes a single value to port
int write_to_kbd(port_t port,unsigned long value)
{
	int attempts = 0;
	unsigned long stat;
	unsigned long response;
	do
	{

		if(attempts >= MAX_ATTEMPTS){
			printf("write_to_kbd() failed, max attempts reached: %d\n", attempts);
			return -1;
		}
		if(sys_inb(STAT_REG, &stat)!= OK){
			printf("write_to_kbd() failed, failure reading status\n");
			return -1;
		}
		if( (stat & IN_BUF_STATUS) == 0 ) {
			if(sys_outb(port, value) != OK){
				printf("write_to_kbd() failed, failure writing value 0x%x, at port 0x%x\n", value, port);
				return -1;
			}
			read_kbd_value(&response);
			if(response == ACK || response == ERROR)
				break;
		}
		attempts++;
		WAIT_MS(WAIT_TIME);
	}while(response == RESEND);
	return response;
}
//sends command with optional argument. hasargs indicates whether argument should be written
int issue_kbd_command(port_t port,unsigned long cmd, unsigned long arg, char hasargs)//issues a commad to the keyboard controller
{
	int attempts = 0;
	unsigned long response;
	do
	{

		if(attempts >= MAX_ATTEMPTS)
			return 1;
		if ((response = write_to_kbd(port,cmd))== -1)//write command
		{
			printf("issue_kbd_command() failed, error writing command\n");
			return 1;
		}
		if(response != ERROR)
		{
			if(hasargs)
			{
				if ((response = write_to_kbd(port,arg))== -1){//if command has arguments, write arguments
					printf("issue_kbd_command() failed, error writing argument\n");
					return 1;
				}
			}
		}
		attempts++;
	} while(response == ERROR);
	return 0;
}
//function that should be executed when a timer interrupt is received in test_leds
int leds_on_timer_int(unsigned long * time, unsigned long* setleds)
{
	if(*time == 0)
	{
		unsigned long unused;
		read_kbd_value(&unused);
		printf("Turning off LEDs\n");
		issue_kbd_command(IN_BUF,LEDS_CMD, *setleds, 1);
	}
	else if((*time % TEST_INT_FREQ) == 0)//if a second has passed
	{
		switch(*ledsptr){
		case 0:
			printf("Toggling Scroll Lock\n");
			break;
		case 1:
			printf("Toggling Num Lock\n");
			break;
		case 2:
			printf("Toggling Caps Lock\n");
			break;
		default:
			break;
		}

		TOGGLE_BIT( (*setleds) ,*(ledsptr++));// toggle led

		if(issue_kbd_command(IN_BUF,LEDS_CMD, *setleds, 1))//send command
		{
			printf("Failure issuing LED command\n");
			return 1;
		}
	}
	(*time)++;
	return 0;
}

int inc_on_timer_int(unsigned long* ticks,unsigned long *unused)
{
	if(ticks == NULL)
		return 1;
	(*ticks)++;
	if((*ticks) % TEST_INT_FREQ == 0) printf("%d out of %d seconds\n", (*ticks) / TEST_INT_FREQ, numseconds);//print every second
	return 0;
}
char timed_condition(unsigned long ticks, unsigned long unused)
{
	return ticks < (numseconds) * TEST_INT_FREQ;
}
int kbd_test_leds_internal(unsigned short n, unsigned short *leds)
{
	numseconds = n+1;
	ledsptr = leds;
	unsigned long ticks = 0;
	unsigned long setleds = 0;
	if(generic_interrupt_loop(&ticks, &setleds, leds_on_timer_int, NULL, 1, timed_condition, NULL))
		return 1;
	return 0;
}

int kbd_test_timed_scan_internal(unsigned short seconds)
{
	numseconds = seconds;
	unsigned long scancode;
	unsigned long ticks = 0;
	if(generic_interrupt_loop(&ticks, &scancode, inc_on_timer_int, scan_on_kbd_int_c, 0, timed_condition, scan_condition))
		return 1;
	return 0;
}
