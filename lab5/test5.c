
#include <stdlib.h>
#include <time.h>
#include "test5.h"
#include "mouse.h"
#include <math.h>
#include<stdio.h>
void *test_init(unsigned short mode, unsigned short delay) {
	void* video_mem = NULL;
	if((video_mem=vg_init(mode)) == NULL)
	{
		printf("test_init(): vg_init() failed");
		return NULL;
	}
	int failure=0;
	int timer_irq_set=0;
	if((timer_irq_set = timer_subscribe_int()) < 0){//subscribe timer 0 interrupts
		printf("test_move(): timer_subscribe_int() failed \n");
		failure = 1;
	}
	int ipc_status;
	message msg;
	int r;
	//unsigned long ticks=0;
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
					if(msg.NOTIFY_ARG & timer_irq_set){
						timer_int_handler();
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		} while(get_ticks() < delay * 60);
	}
	if(timer_unsubscribe_int()){//unsubscribe interrupts
		printf("test_move(): timer_unsubscribe_int() failed\n");
		failure = 1;
	}
	if(vg_exit()){
		printf("test_init(): vg_exit() failed");
		return NULL;
	}
	printf("Physical Address of VRAM: 0x%X\n", get_vram_phys_addr());
	return (void*)get_video_mem();
}


int test_square(unsigned short x, unsigned short y, unsigned short size, unsigned long color) {
	char* video_mem = NULL;
	int failure =0;
	if((video_mem=(char *)vg_init(VBE_MODE_RES_1024x768)) == NULL)
	{
		printf("test_init(): vg_init() failed");
		return 1;
	}
	
	if(draw_square(video_mem,x,y,size, color))
		failure =1;
	int kbd_irq_set = 0;
	if((kbd_irq_set = kbd_subscribe_int(0)) < 0){//subscribe kbd interrupts
		printf("generic_interrupt_loop(): kbd_subscribe_int() failed \n");
		failure = 1;
	}
	int ipc_status;
	message msg;
	int r;
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
					if (msg.NOTIFY_ARG & kbd_irq_set) { /* subscribed kbd interrupt */
						if(kbd_int_handler())
							failure = 1;
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		} while(get_scancode() != ESC_BREAK);
	}
	if(kbd_unsubscribe_int()){//unsubscribe interrupts
		printf("test_square(): kbd_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");
	if(vg_exit()){
		printf("test_square(): vg_exit() failed");
		return 1;
	}
	return failure;
}

int test_circle(unsigned short x, unsigned short y, unsigned short radius, unsigned long color) {
	char* video_mem = NULL;
	int failure =0;
	if((video_mem=(char *)vg_init(VBE_MODE_RES_1024x768)) == NULL)
	{
		printf("test_init(): vg_init() failed");
		return 1;
	}

	if(draw_circle(video_mem,x,y,radius, color))
		failure =1;
	int kbd_irq_set = 0;
	if((kbd_irq_set = kbd_subscribe_int(0)) < 0){//subscribe kbd interrupts
		printf("generic_interrupt_loop(): kbd_subscribe_int() failed \n");
		failure = 1;
	}
	int ipc_status;
	message msg;
	int r;
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
					if (msg.NOTIFY_ARG & kbd_irq_set) { /* subscribed kbd interrupt */
						if(kbd_int_handler())
							failure = 1;
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		} while(get_scancode() != ESC_BREAK && !failure);
	}
	if(kbd_unsubscribe_int()){//unsubscribe interrupts
		printf("test_square(): kbd_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");
	if(vg_exit()){
		printf("test_square(): vg_exit() failed");
		return 1;
	}
	return failure;
}


int test_line(unsigned short xi, unsigned short yi, 
		           unsigned short xf, unsigned short yf, unsigned long color) {
	
	char* video_mem = NULL;
	int failure =0;
	if((video_mem=(char *)vg_init(VBE_MODE_RES_1024x768)) == NULL)
	{
		printf("test_line(): vg_init() failed");
		return 1;
	}
	draw_line(video_mem,xi,yi, xf,yf,color);
	int kbd_irq_set = 0;
	if((kbd_irq_set = kbd_subscribe_int(0)) < 0){//subscribe kbd interrupts
		printf("test_line(): kbd_subscribe_int() failed \n");
		failure = 1;
	}
	int ipc_status;
	message msg;
	int r;
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
					if (msg.NOTIFY_ARG & kbd_irq_set) { /* subscribed kbd interrupt */
						if(kbd_int_handler())
							failure = 1;
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		} while(get_scancode() != ESC_BREAK && !failure);
	}
	if(kbd_unsubscribe_int()){//unsubscribe interrupts
		printf("test_square(): kbd_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");

	if(vg_exit()){
		printf("test_line(): vg_exit() failed");
		return 1;
	}
	return failure;
	
}

int test_xpm(unsigned short xi, unsigned short yi, char *xpm[]) {
	if(xpm == NULL)
		return 1;
	char* video_mem = NULL;
	int failure =0;
	if((video_mem=(char *)vg_init(VBE_MODE_RES_1024x768)) == NULL)
	{
		printf("test_xpm(): vg_init() failed");
		return 1;
	}
	draw_xpm(video_mem,xi, yi,xpm);
	int kbd_irq_set = 0;
	if((kbd_irq_set = kbd_subscribe_int(0)) < 0){//subscribe kbd interrupts
		printf("generic_interrupt_loop(): kbd_subscribe_int() failed \n");
		failure = 1;
	}
	int ipc_status;
	message msg;
	int r;
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
					if (msg.NOTIFY_ARG & kbd_irq_set) { /* subscribed kbd interrupt */
						if(kbd_int_handler())
							failure = 1;
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		} while(get_scancode() != ESC_BREAK && !failure);
	}
	if(kbd_unsubscribe_int()){//unsubscribe interrupts
		printf("test_square(): kbd_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");
	if(vg_exit()){
		printf("test_xpm(): vg_exit() failed");
		return 1;
	}
	return failure;
}	

int test_move(unsigned short xi, unsigned short yi, char *xpm[], 
				unsigned short hor, short delta, unsigned short time1)
{
	if(xpm == NULL)
		return 1;
	char* video_mem = NULL;
	int failure =0;
	int width, height;
	char* pix = NULL;
	if((pix = (char*) read_xpm(xpm, &width, &height)) == NULL)
		return 1;
	if((video_mem=(char *)vg_init(VBE_MODE_RES_1024x768)) == NULL)
	{
		printf("test_move(): vg_init() failed");
		return 1;
	}
	int kbd_irq_set = 0;
	int timer_irq_set=0;
	if((kbd_irq_set = kbd_subscribe_int(0)) < 0){//subscribe kbd interrupts
		printf("test_move(): kbd_subscribe_int() failed \n");
		failure = 1;
	}
	if((timer_irq_set = timer_subscribe_int()) < 0){//subscribe timer 0 interrupts
		printf("test_move(): timer_subscribe_int() failed \n");
		failure = 1;
	}
	int ipc_status;
	message msg;
	int r;
	double step = (double)delta/(time1*60);
	double xfp = xi;
	double yfp = yi;
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
					if (msg.NOTIFY_ARG & kbd_irq_set)  /* subscribed kbd interrupt */
						if(kbd_int_handler())
							failure = 1;
					if(msg.NOTIFY_ARG & timer_irq_set){
						if(get_ticks() < time1 * 60){//enquanto tempo desjado nao passou
							timer_int_handler();
							blank_secondary_buf();//clear buffer
							draw_pixmap(get_secondary_buf(),(int) xfp, (int) yfp, height, width, pix);
							commit_to_video_mem();
							if(hor)
								xfp += step;//se movimento horizontal incrementar x
							else yfp +=step;//senao incrementar y
						}
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
		} while(get_scancode() != ESC_BREAK && !failure);
	}
	if(timer_unsubscribe_int()){//unsubscribe interrupts
		printf("test_move(): timer_unsubscribe_int() failed\n");
		failure = 1;
	}
	if(kbd_unsubscribe_int()){//unsubscribe interrupts
		printf("test_move(): kbd_unsubscribe_int() failed\n");
		failure = 1;
	}
	printf("Done\n");
	if(vg_exit()){
		printf("test_move(): vg_exit() failed");
		return 1;
	}
	return failure;
}					
int test_controller()
{
	vbe_controller_info_t info;
	void* base=NULL;
	if(vbe_get_controller_info(&info, &base)){
		printf("test_controller(): vbe_get_controller_info() failed\n");
		return 1;
	}
	printf("VBE Signature        : %s\n", info.VbeSignature);
	printf("VBE Version          : ");print_vbe_version(info.VbeVersion); printf("\n");
	printf("OEM String           : %s\n", GET_ADDR(info.OemStringPtr, base));
	printf("Total VRAM memory    : %u (blocks of 64 kb)\n", info.TotalMemory);
	printf("OEM Software Revision: "); print_vbe_version(info.OemSoftwareRev); printf("\n");
	printf("OEM Vendor Name      : %s\n",  GET_ADDR(info.OemVendorNamePtr, base),GET_ADDR(info.OemVendorNamePtr, base));
	printf("OEM Product Name     : %s\n", GET_ADDR(info.OemProductNamePtr, base));
	printf("OEM Product Revision : %s\n", GET_ADDR(info.OemProductRevPtr, base));
	unsigned long capabilities = (unsigned long) (*(info.Capabilities));
	printf("Capabilities: 0x%x\n",capabilities);
	if((capabilities & BIT(0)) == 0)
		printf("\tDAC is fixed width, with 6 bits per primary color\n");
		else  printf("\tDAC width is switchable to 8 bits per primary color\n");
	if((capabilities & BIT(1)) == 0)
		printf("\tController is VGA compatible\n");
		else printf("\tController is not VGA compatible\n");
	if((capabilities & BIT(2)) == 0)
		printf("\tNormal RAMDAC operation\n");
		else  printf("\tRAMDAC operation: Blank bit in Function 09h\n");
	short* modes = GET_ADDR(info.VideoModePtr, base);
	size_t nummodes=0;
	printf("Supported modes:\n");
	while(*modes != -1)
	{
		nummodes++;
		printf("Mode number %3d : 0x%X\n", nummodes,*modes);
		modes++;
	}
	if(nummodes == 0)
		printf("No modes supported\n");
	return 0;
}

