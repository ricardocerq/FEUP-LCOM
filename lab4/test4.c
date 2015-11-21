#include "test4.h"
#include "mouse.h"
#include "ps2mouse.h"
#include "vector.h"
#include <minix/drivers.h>

unsigned short packetcount = 0;

int test_packet(unsigned short cnt){
	if(cnt == 0)
		return 1;
	int mouse_irq_set = 0;
	char failure = 0;//set to 1 if something fails
	if((mouse_irq_set = mouse_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("test_packet(): mouse_subscribe_int() failed \n");
		failure = 1;
	}
	short packet[3];
	size_t packetindex=0;
	int start = 1;
	int ipc_status;
	message msg;
	int r;
	if(ISSUE_MOUSE_CMD_NO_ARGS(SET_STREAM_MODE))
	{
		printf("Failure setting stream mode\n");
		failure = 1;
	}
	if(ISSUE_MOUSE_CMD_NO_ARGS(ENABLE_DATA_PACKETS))
	{
		printf("Failure enabling data packets\n");
		failure = 1;
	}
	if(!failure){
		while(packetcount < cnt){
			/* Get a request message. */
			if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & mouse_irq_set) { /* subscribed timer interrupt */
						//printf("writing to position: %d\n", packetindex);
						if(start)
							start = 0;
						else{if(read_mouse_value(packet+packetindex))
						{
							printf("Action on mouse interrupt failed\n");
							failure = 1;
						}
						packetindex++;
						processpacket(packet, &packetindex, &packetcount);}
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		}
	}
	ISSUE_MOUSE_CMD_NO_ARGS(DISABLE_DATA_PACKETS);
	if(mouse_unsubscribe_int()){//unsubscribe interrupts
		printf("test_packet(): mouse_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");
	return failure;
}	
	
int test_async(unsigned short idle_time) {
	if(idle_time == 0)
		return 1;
	int mouse_irq_set = 0;
	int timer_irq_set = 0;
	char failure = 0;//set to 1 if something fails
	if((mouse_irq_set = mouse_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("test_packet(): mouse_subscribe_int() failed \n");
		failure = 1;
	}
	if((timer_irq_set = timer_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("test_packet(): timer_subscribe_int() failed \n");
		failure = 1;
	}
	short packet[3];
	size_t packetindex=0;
	int start = 1;
	int ipc_status;
	message msg;
	int r;
	unsigned long ticks= 0;
	if(ISSUE_MOUSE_CMD_NO_ARGS(SET_STREAM_MODE))
	{
		printf("Failure setting stream mode\n");
		failure = 1;
	}
	if(ISSUE_MOUSE_CMD_NO_ARGS(ENABLE_DATA_PACKETS))
	{
		printf("Failure enabling data packets\n");
		failure = 1;
	}
	if(!failure){
		while(ticks < idle_time*60){
			/* Get a request message. */
			if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & mouse_irq_set) { /* subscribed timer interrupt */
						//printf("writing to position: %d\n", packetindex);
						ticks = 0;
						if(start)
							start = 0;
						else{if(read_mouse_value(packet+packetindex))
						{
							printf("Action on mouse interrupt failed\n");
							failure = 1;
						}
						packetindex++;
						processpacket(packet, &packetindex, &packetcount);}
					}
					if (msg.NOTIFY_ARG & timer_irq_set) { /* subscribed timer interrupt */
						ticks++;
						if(ticks % 60 == 0)
							printf("%d out of %d seconds\n",ticks / 60 , idle_time);
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		}
	}
	if(ISSUE_MOUSE_CMD_NO_ARGS(DISABLE_DATA_PACKETS))
	{
		printf("Failure disabling data packets\n");
		failure = 1;
	}
	if(mouse_unsubscribe_int()){//unsubscribe interrupts
		printf("test_packet(): mouse_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");
	return failure;
}
	
int test_config(void) {
	int mouse_irq_set = 0;
	char failure = 0;//set to 1 if something fails
	if((mouse_irq_set = mouse_subscribe_int()) < 0){//subscribe mouse interrupts
		printf("test_packet(): mouse_subscribe_int() failed \n");
		failure = 1;
	}
	unsigned short status[3];
	if(ISSUE_MOUSE_CMD_NO_ARGS(STATUS_REQUEST))//send status request
	{
		printf("failure issuing status request\n");
		failure = 1;
	}
	size_t i = 0;
	for(; i < 3; ++i)
	{
		read_mouse_value(status+i);
		//printf("BYTE %d: %x\n", i, status[i]);
	}
	display_status(status);
	if(mouse_unsubscribe_int()){//unsubscribe interrupts
		printf("test_packet(): mouse_unsubscribe_int() failed\n");
		failure = 1;
	}
	return failure;
}	
	
int test_gesture(short length, unsigned short tolerance) {
	if(length == 0 || tolerance == 0)
		return 1;
	int mouse_irq_set = 0;
	char failure = 0;//set to 1 if something fails
	if((mouse_irq_set = mouse_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("test_gesture(): mouse_subscribe_int() failed \n");
		failure = 1;
	}
	short packet[3];
	size_t packetindex=0;
	int start = 1;
	int ipc_status;
	message msg;
	int r;
	if(ISSUE_MOUSE_CMD_NO_ARGS(SET_STREAM_MODE))
	{
		printf("Failure setting stream mode\n");
		failure = 1;
	}
	if(ISSUE_MOUSE_CMD_NO_ARGS(ENABLE_DATA_PACKETS))
	{
		printf("Failure enabling data packets\n");
		failure = 1;
	}
	event_t ev;
	mouse_state_t mouse;
	initMouseState (&mouse);
	initEvent(&ev);
	vector_t * trajectories = (vector_t *) new_vector_t(sizeof(coord_t)); // vector of coordinates, to keep a record of all trajectories that could be horizontal
	if(!failure){
		while(1){
			/* Get a request message. */
			if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & mouse_irq_set) { /* subscribed timer interrupt */
						if(start)
							start = 0;
						else{if(read_mouse_value(packet+packetindex))
						{
							printf("Action on mouse interrupt failed\n");
							failure = 1;
						}
						//printf("%x ",*(packet+packetindex));
						packetindex++;
						processpacket(packet, &packetindex, &packetcount);}
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
			if(packetindex == 0)
			{
				getEvent2(&ev,&mouse,packet);
				if(check_hor_line2(&ev,length,tolerance, trajectories))
					break;
			}
		}
	}
	if(ISSUE_MOUSE_CMD_NO_ARGS(DISABLE_DATA_PACKETS))
	{
		printf("Failure disabling data packets\n");
		failure = 1;
	}
	if(mouse_unsubscribe_int()){//unsubscribe interrupts
		printf("test_gesture(): mouse_unsubscribe_int() failed\n");
		failure = 1;
	}
	delete_vector_t(trajectories);
	printf("Done\n");
	return failure;
}
