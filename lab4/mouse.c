#include "mouse.h"
#include "kbd.h"
#include "timer.h"
#include "kbd.h"
#include "i8042.h"
#include "vector.h"

static int mouse_hook_id = ORIGINAL_KBD_HOOK_ID;
static int numseconds = 0;

//subscribe mouse interrupts
int mouse_subscribe_int(void)
{
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
	//printf("resetting coords\n");
	c->x = 0;
	c->y = 0;
}
//initialize event
void initEvent(event_t* e)
{
	resetCoord(&(e->delta));
	e->type = LUP;
}
//initialize mouse state
void initMouseState(mouse_state_t* m)
{
	resetCoord(&(m->coords));
	m->lb =0;
	m->mb = 0;
	m->rb = 0;
}

//if x of coordinate 1 and 2 have different signs return is (0,0)
coord_t addCoordRes(coord_t c1, coord_t c2)
{
	coord_t out;
	if(c1.x * c2.x < 0)
	{
		resetCoord(&out);
		return out;
	}
	out.x = c1.x + c2.x;
	out.y = c1.y + c2.y;
	return out;
}
// add two coordinates
coord_t addCoord(coord_t c1, coord_t c2)
{
	coord_t out;
	out.x = c1.x + c2.x;
	out.y = c1.y + c2.y;
	return out;
}
void printCoord(coord_t c)
{
	printf("x: %d, y: %d\n", c.x, c.y);
}

//unused
void getEvent(event_t* ev, mouse_state_t* m, short * packet)
{
	int lb = GET_BIT_NUM(packet[0], L_B);
	if (XOR(m->lb, lb))
	{
		m->lb = lb;
		if(lb == 1)
			ev->type = LDOWN;
		else ev->type = LUP;
		ev->delta = getMousexy(packet);
	}
	else
	{
		ev->type = MOVE;
		coord_t c;
		c = getMousexy(packet);
		ev->delta = addCoordRes(ev->delta, c);
	}
}
void getEvent2(event_t* ev, mouse_state_t* m, short * packet)
{
	int lb = GET_BIT_NUM(packet[0], L_B);
	if (XOR(m->lb, lb)) //state of left button changed
	{
		m->lb = lb;//new state of button
		if(lb == 1)
			ev->type = LDOWN; //button was pressed
		else ev->type = LUP; //button was released
		ev->delta = getMousexy(packet);
	}
	else
	{
		ev->type = MOVE;//if the button didnt change the mouse was moved
		coord_t c;
		ev->delta = getMousexy(packet);
	}
}

//unused
int check_hor_line(event_t* evt,short length, unsigned short tolerance) {
	static state_t state = INIT; // initial state; keep state
	printf("state: %d\n", state);
	switch (state) {
	case INIT:
		if( evt->type == LDOWN )
			state = DRAW;
		break;
	case DRAW:
		if( evt->type == MOVE ) {// need to check if HOR_LINE event occurs
			if(abs(evt->delta.y) > tolerance)
				resetCoord(&(evt->delta));
			else if( length > 0)
				{if(evt->delta.x > length)
					return 1;}
			else if(length < 0)
				{if(evt->delta.x < length)
					return 1;}
		} else if( evt->type == LUP )
			state = INIT;
		break;
	default:
		break;
	}
	return 0;
}

//add delta to all elements of vector, add delta as element of vector, return 1 if vector has at least 1 horizontal movement else return 0
int verify(vector_t* trajectories,event_t* evt, short length, unsigned short tolerance)
{
	if((evt->delta.x == 0) && (evt->delta.y == 0))//no movement
		return 0;
	size_t i  = 0;
	for(; i < size(trajectories); i++)
	{
		if( ((coord_t *) at(trajectories, i))->x * evt->delta.x < 0) //if direction in x axis is reversed...
			{
				erase(trajectories, i);// ...cancel current trajectory
				i--;
			}
		else
		{
			(* (coord_t *) at(trajectories, i)) = addCoord(*(coord_t *)(at(trajectories, i)), evt->delta);//add last movement to each trajectory
			if(abs( ((coord_t *)at(trajectories, i))->y) > tolerance) //if movement in y axis exceeds tolerance...
			{
				erase(trajectories, i); //...cancel trajectory
				i--;
			}
			else
			{
				if( length > 0)
				{if(  ((coord_t *)(at(trajectories, i)))->x > length)
					return 1;} // if at least one trajectory exceeds length, the movement is horizontal
				else if(length < 0)
				{if(  ((coord_t *)(at(trajectories, i)))->x < length)
					return 1;}// if at least one trajectory exceeds length, the movement is horizontal
			}
		}
	}
	push_back(trajectories, &(evt->delta)); // each movement is a new trajectory
	return 0;
}

//returns true if movement is horizontal
int check_hor_line2(event_t* evt,short length, unsigned short tolerance, vector_t* trajectories) {
	static state_t state = INIT; // initial state; keep state
	//printf("state: %d\n", state);
	switch (state) {
	case INIT:
		if( evt->type == LDOWN )
			state = DRAW;
		clear(trajectories);//reset trajectories
		break;
	case DRAW:
		if( evt->type == MOVE ) {
			if(verify(trajectories,evt, length, tolerance))
				return 1;//is horizontal movement
		} else if( evt->type == LUP )
			state = INIT;
		break;
	default:
		break;
	}
	return 0;//not horizontal movement
}
