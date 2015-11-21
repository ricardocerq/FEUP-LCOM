#include "mouse.h"
#include "kbd.h"
#include "timer.h"
#include "kbd.h"
#include "i8042.h"
#include "vector.h"

static int mouse_hook_id = ORIGINAL_MOUSE_HOOK_ID;
static int numseconds = 0;
static mouse_state_t mouse;
static short packet[3];
static size_t packetindex=0;
static unsigned short packetcount=0;
static int updated = 0;//indicates if mouse has been updated since it was last accessed


void assign_mouse(mouse_state_t* m1, mouse_state_t* m2)
{
	memcpy(m1, m2, sizeof(mouse_state_t));
}

void dump_packets(void)
{
	display_packet(packet);
}

static void update_mouse_state(void)
{
	mouse.lb = GET_BIT_NUM(packet[0], L_B);
	mouse.rb = GET_BIT_NUM(packet[0], R_B);
	mouse.mb = GET_BIT_NUM(packet[0], M_B);
	mouse.coords = getMousexy(packet);
	updated = 1;
}

int mouse_is_updated(void)
{
	return updated;
}
mouse_state_t* get_mouse_state(void)
{
	updated = 0;
	return &mouse;
}
int mouse_int_handler(void)
{
	static int start=1;
	if(start > 0)
		start--;
	else
	{
		unsigned long value;
		if(read_mouse_value(&value))
		{
			printf("Action on mouse handler failed\n");
			return 1;
		}
		*(packet+packetindex) = (short) value;
		packetindex++;
		processpacket_no_display(packet, &packetindex, &packetcount);
		if(packetindex == 0) //packet has been processed
			update_mouse_state();
	}
	return 0;
}

int clear_buf()
{
	unsigned long i;
	while(!read_kbd_value(&i))
	{};
	return 0;
}
//initialize mouse state
void initMouseState(mouse_state_t* m)
{
	updated= 0;
	resetCoord(&(m->coords));
	m->lb =0;
	m->mb = 0;
	m->rb = 0;
}
//subscribe mouse interrupts
int mouse_subscribe_int(void)
{
	initMouseState(&mouse);
	int returnvalue = BIT(mouse_hook_id);
	int irqpolicy = IRQ_EXCLUSIVE | IRQ_REENABLE;
	if(sys_irqsetpolicy(MOUSE_IRQ, irqpolicy, &mouse_hook_id)!= OK){
		printf("mouse_subscribe_int(): sys_irqsetpolicy() failed\n");
		return -1;
	}

	if(sys_irqenable(&mouse_hook_id)!=OK){
		printf("mouse_subscribe_int(): sys_irqenable() failed\n");
		return -1;
	}
	return returnvalue;//return value with bit number (original hook_id) set to 1
}
////unsubscribe mouse interrupts
int mouse_unsubscribe_int(void) {
	if(sys_irqdisable(&mouse_hook_id) != OK){
		printf("mouse_unsubscribe_int(): sys_irqdisable() failed\n");
		return 1;
	}
	if(sys_irqrmpolicy(&mouse_hook_id) != OK){
		printf("mouse_unsubscribe_int(): sys_irqrmpolicy() failed\n");
		return 1;
	}
	return 0;
}
//get mouse delta
coord_t getMousexy(short * packet)
{
	coord_t out;
	if(GET_BIT_NUM(packet[0], X_SIGN))
		out.x = (char) (packet[1]); // is negative
	else out.x = (int) (packet[1]); // is positive
	if(GET_BIT_NUM(packet[0], Y_SIGN))
		out.y = (char) (packet[2]);//is negative
	else out.y = (int) (packet[2]);//is positive
	return out;
}

void display_packet(short * packet)
{
	printf("B1=0x%02x ", packet[0]);//byte 1
	printf("B2=0x%02x ", packet[1]);//byte 2
	printf("B3=0x%02x ", packet[2]);//byte 3
	printf("LB=%d ", GET_BIT_NUM(packet[0], L_B));
	printf("MB=%d ", GET_BIT_NUM(packet[0], M_B));
	printf("RB=%d ", GET_BIT_NUM(packet[0], R_B));
	printf("XOV=%d ", GET_BIT_NUM(packet[0], X_OVF));
	printf("YOV=%d ", GET_BIT_NUM(packet[0], Y_OVF));
	coord_t mousecoordinates = getMousexy(packet);
	printf("X=%4d ", mousecoordinates.x);
	printf("Y=%4d\n", mousecoordinates.y);
}


int processpacket(short* packet, size_t* packetindex, unsigned short* packetcount)
{
	if(!(IS_BYTE_1(packet[0])))
		{
			(*packetindex)--;//throw out packet
		}
	if((*packetindex) == 3)
	{
		display_packet(packet);
		(*packetindex)=0;
		(*packetcount)++;
	}
	return 0;
}

int processpacket_no_display(short* packet, size_t* packetindex, unsigned short* packetcount)
{
	if(!(IS_BYTE_1(packet[0])))
		{
			(*packetindex)--;//throw out packet
		}
	if((*packetindex) == 3)
	{
		(*packetindex)=0;
		(*packetcount)++;
	}
	return 0;
}
//write a value to kbc without resending
int write_to_kbc_no_resend(port_t port,unsigned long value, unsigned char read_response)
{
	int attempts = 0;
	unsigned long stat;
	unsigned long response=0;
	while(1)
	{
		if(attempts >= MAX_ATTEMPTS){
			printf("write_to_kbc_no_resend() failed, max attempts reached: %d\n", attempts);
			return -1;
		}
		if(sys_inb(STAT_REG, &stat)!= OK){
			printf("write_to_kbc_no_resend() failed, failure reading status\n");
			return -1;
		}
		if( (stat & IN_BUF_STATUS) == 0 ) {
			if(sys_outb(port, value) != OK){
				printf("write_to_kbc_no_resend() failed, failure writing value 0x%x, at port 0x%x\n", value, port);
				return -1;
			}
			if(read_response)
			read_kbd_value(&response);
			break;
		}
		attempts++;
		WAIT_MS(WAIT_TIME);
	}
	return response;
}
//write a value to the mouse
int write_to_mouse(unsigned long value)
{
	int response = 0;
	if((response = write_to_kbc_no_resend(KBC_CMD_REG,WRITE_BYTE_MOUSE,0)) == -1)
		return 1;
	response = 0;
	if((response = write_to_kbc_no_resend(IN_BUF,value,1)) == -1)
		return 1;
	return response;
}
int read_mouse_value(unsigned long* data)
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
int issue_mouse_command(unsigned long cmd, unsigned long arg, char hasargs)//issues a commad to the keyboard controller
{
	int attempts = 0;

	unsigned long response;
	while(1)
	{

		if(attempts >= MAX_ATTEMPTS)
			return 1;
		if ((response = write_to_mouse(cmd))== -1)//write command
		{
			printf("issue_mouse_command() failed, error writing command\n");
			return 1;
		}
		if(response != ACK)
			{attempts++;continue;}
		if(hasargs)
		{
			if ((response = write_to_mouse(arg))== -1){//if command has arguments, write arguments
				printf("issue_mouse_command() failed, error writing argument\n");
				return 1;
			}
		}
		if(response != ACK)
		{attempts++;continue;}
		if(response == ACK)
			break;
	}
	return 0;
}

void display_status(unsigned short* status)
{
	if(status[0] & (REMOTE_BIT))
		printf("Remote (polled) mode\n");
	else {
		printf("Stream mode\n");
		if(status[0] & (DATA_REPORTING_BIT))
			printf("Data reporting enabled\n");
		else printf("Data reporting disabled\n");
	}
	if(status[0] & (SCALING_BIT))
		printf("Scaling is 2:1\n");
	else printf("Scaling is 1:1\n");
	if(status[0] & (L_B))
		printf("Left button is currently pressed\n");
	else printf("Left button is currently released\n");
	if(status[0] & (M_B))
			printf("Middle button is currently pressed\n");
		else printf("Middle button is currently released\n");
	if(status[0] & (R_B))
			printf("Right button is currently pressed\n");
		else printf("Right button is currently released\n");
	printf("Resolution: %d\n", status[1]&(RESOLUTION_BITS));
	printf("Sample rate: %d\n", status[2]);
}

//set a coordinate to (0,0)
void resetCoord(coord_t * c)
{
	c->x = 0;
	c->y = 0;
}
