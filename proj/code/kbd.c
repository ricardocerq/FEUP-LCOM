#include "kbd.h"
#include "timer.h"
#include "i8042.h"
#include <stdbool.h>

#define DELAY_US 20000
#define TEST_INT_FREQ 41 //is a divisor of TIMER_FREQ
#define DEFAULT_FREQ 60

static int kbd_hook_id = ORIGINAL_KBD_HOOK_ID;
static int numseconds = 0;
static unsigned short* ledsptr = NULL;
static unsigned long scancode = 0;
static char firstbyte = 0;
static bool shiftdown = false;
static bool altdown = false;
static bool ctrldown = false;

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

int kbd_subscribe_int()
{
	int returnvalue = BIT(kbd_hook_id);
	if(sys_irqsetpolicy(KBD_IRQ, IRQ_EXCLUSIVE | IRQ_REENABLE, &kbd_hook_id)!= OK){
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

char scancode_default[][2] =
{
		{0x2, '1'},
		{0x3, '2'},
		{0x4, '3'},
		{0x5, '4'},
		{0x6, '5'},
		{0x7, '6'},
		{0x8, '7'},
		{0x9, '8'},
		{0xa, '9'},
		{0xb, '0'},
		{0x1b, 39},//
		{0xc, '\''},
		{0x29, '\\'},
		{0x1a, '+'},
		{0x33, ','},
		{0x34, '.'},
		{0x35, '-'},
		{0x56, '<'},
		{0x1e, 'a'},
		{0x30, 'b'},
		{0x2e, 'c'},
		{0x20, 'd'},
		{0x12, 'e'},
		{0x21, 'f'},
		{0x22, 'g'},
		{0x23, 'h'},
		{0x17, 'i'},
		{0x24, 'j'},
		{0x25, 'k'},
		{0x26, 'l'},
		{0x32, 'm'},
		{0x31, 'n'},
		{0x18, 'o'},
		{0x19, 'p'},
		{0x10, 'q'},
		{0x13, 'r'},
		{0x1f, 's'},
		{0x14, 't'},
		{0x16, 'u'},
		{0x2f, 'v'},
		{0x11, 'w'},
		{0x2d, 'x'},
		{0x15, 'y'},
		{0x2c, 'z'},
		{0x2b, '~'},
		{0x39, 32}//space
};
///shift
char scancode_shift[][2] =
{
		{0x2, '!'},
		{0x3, '\"'},
		{0x4, '#'},
		{0x5, '$'},
		{0x6, '%'},
		{0x7, '&'},
		{0x8, '/'},
		{0x9, '('},
		{0xa, ')'},
		{0xb, '='},
		{0x1b, 96},//
		{0xc, '?'},
		{0x29, '|'},
		{0x1a, '*'},
		{0x33, ';'},
		{0x34, ':'},
		{0x35, '_'},
		{0x56, '>'},
		{0x1e, 'A'},
		{0x30, 'B'},
		{0x2e, 'C'},
		{0x20, 'D'},
		{0x12, 'E'},
		{0x21, 'F'},
		{0x22, 'G'},
		{0x23, 'H'},
		{0x17, 'I'},
		{0x24, 'J'},
		{0x25, 'K'},
		{0x26, 'L'},
		{0x32, 'M'},
		{0x31, 'N'},
		{0x18, 'O'},
		{0x19, 'P'},
		{0x10, 'Q'},
		{0x13, 'R'},
		{0x1f, 'S'},
		{0x14, 'T'},
		{0x16, 'U'},
		{0x2f, 'V'},
		{0x11, 'W'},
		{0x2d, 'X'},
		{0x15, 'Y'},
		{0x2c, 'Z'},
		{0x2b, '^'},
		{0x39, 32}//space
};
///altctrl
char scancode_alt_ctrl[][2] =
{
		{0x3, '@'},
		{0x8, '{'},
		{0x9, '['},
		{0xa, ']'},
		{0xb, '}'},
		{0x39, 32} // space
};
int process_input(unsigned long scancode, char* character)
{
	switch(scancode)
	{
	case LEFT_SHIFT_MAKE:
		shiftdown = true;
		return IGNORE_INPUT;
		break;
	case RIGHT_SHIFT_MAKE:
		shiftdown = true;
		return IGNORE_INPUT;
		break;
	case LEFT_CTRL_MAKE:
		ctrldown = true;
		return IGNORE_INPUT;
		break;
	case RIGHT_CTRL_MAKE:
		ctrldown = true;
		return IGNORE_INPUT;
		break;
	case LEFT_ALT_MAKE:
		altdown = true;
		return IGNORE_INPUT;
		break;
	case ALT_GR_MAKE:
		ctrldown = true;
		altdown = true;
		return IGNORE_INPUT;
		break;

	case (LEFT_SHIFT_MAKE | BREAK_CODE_BIT):
	shiftdown = false;
	return IGNORE_INPUT;
	break;
	case (RIGHT_SHIFT_MAKE | BREAK_CODE_BIT):
	shiftdown = false;
	return IGNORE_INPUT;
	break;
	case (LEFT_CTRL_MAKE | BREAK_CODE_BIT):
	ctrldown = false;
	return IGNORE_INPUT;
	break;
	case (RIGHT_CTRL_MAKE | BREAK_CODE_BIT):
	ctrldown = false;
	return IGNORE_INPUT;
	break;
	case (LEFT_ALT_MAKE | BREAK_CODE_BIT):
	altdown = false;
	return IGNORE_INPUT;
	break;
	case (ALT_GR_MAKE | BREAK_CODE_BIT):
	ctrldown = false;
	altdown = false;
	return IGNORE_INPUT;
	break;

	default:
		break;
	}
	size_t i = 0;
	if(shiftdown)
	{
		for(; i < SHIFT_MAP_NUM_ENTRIES+1; i++)
			if(scancode_shift[i][0] == scancode)
			{
				*character = scancode_shift[i][1];
				return CHAR_RECEIVED;
			}
	}
	else if(altdown && ctrldown)
	{
		for(; i < ALT_CTRL_MAP_NUM_ENTRIES +1; i++)
			if(scancode_alt_ctrl[i][0] == scancode)
			{
				*character = scancode_alt_ctrl[i][1];
				return CHAR_RECEIVED;
			}
	}
	else
	{
		for(; i < DEFAULT_MAP_NUM_ENTRIES +1; i++)
			if(scancode_default[i][0] == scancode)
			{
				*character = scancode_default[i][1];
				return CHAR_RECEIVED;
			}
	}
	return scancode;
}

