#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/video.h>
#include "video_gr.h"
#include "vbe.h"
#include "graphics_mode.h"
#include "mouse.h"
#include "pixmap.h"
#include "i8254.h"
#include <math.h>

/* Constants for VBE 0x105 mode */

/* The physical address may vary from VM to VM.
 * At one time it was 0xD0000000
 *  #define VRAM_PHYS_ADDR    0xD0000000 
 * Currently on lab B107 is 0xF0000000
 * Better run my version of lab5 as follows:
 *     service run `pwd`/lab5 -args "mode 0x105"
 */
#define VRAM_PHYS_ADDR	  0xF0000000
#define H_RES             1024
#define V_RES		  	  768
#define BITS_PER_PIXEL	  8

/* Private global variables */

static char *video_mem=NULL;		/* Process address to which VRAM is mapped */
static char *secondary_buf=NULL;			//double buffer
static unsigned h_res=0;		/* Horizontal screen resolution in pixels */
static unsigned v_res=0;		/* Vertical screen resolution in pixels */
static unsigned bits_per_pixel=0; /* Number of VRAM bits per pixel */
static unsigned bytes_per_pixel=0;
static void* vram_phys_addr=NULL;
// accessor functions for private variables
char* get_video_mem(void)
{
	return video_mem;
}
char* get_secondary_buf(void)
{
	return secondary_buf;
}
unsigned get_h_res(void)
{
	return h_res;
}

unsigned get_v_res(void)
{
	return v_res;
}

unsigned get_bits_per_pixel(void)
{
	return bits_per_pixel;
}

unsigned get_bytes_per_pixel(void)
{
	return bytes_per_pixel;
}
void* get_vram_phys_addr(void)
{
	return vram_phys_addr;
}
int set_video_mode(unsigned short mode)
{
	struct reg86u reg;
	reg.u.w.ax = (VBE_IDENTIFIER<<8)|(VBE_F_SET_MODE); // VBE call, function 02 -- set VBE mode
	reg.u.w.bx = (BIT(LINEAR_MODEL_BIT))|mode; 							// set bit 14: linear framebuffer
	reg.u.b.intno = VIDEO_CARD_INT;
	if( sys_int86(&reg) != OK ) {
		printf("set_vbe_mode: sys_int86() failed \n");
		return 1;
	}
	if(reg.u.b.ah == VBE_FAIL)
	{
		printf("vg_init(): Function call failed\n");
		return 1;
	}
	else if(reg.u.b.ah == VBE_N_SUP)
	{
		printf("vg_init(): Function is not supported in current HW configuration\n");
		return 1;
	}
	else if(reg.u.b.ah == VBE_INVALID)
	{
		printf("vg_init(): Function is invalid in current video mode\n");
		return 1;
	}
	else if(reg.u.b.ah != VBE_OK)
	{
		printf("vg_init(): error setting mode %x", mode);
		return 1;
	}
	return 0;
}
//maps buffer at base base, size size to proccess's virtual memory,
void* map_virtual_memory(unsigned long base, unsigned long size, void** ptr)
{
	int r;
	struct mem_range mr;
	mr.mr_base = (phys_bytes)(base);
	mr.mr_limit = mr.mr_base + size;
	if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		panic("video_txt: sys_privctl (ADD_MEM) failed: %d\n", r);
	*ptr = vm_map_phys(SELF, (void *)mr.mr_base, size);
	if(*ptr == MAP_FAILED)
		panic("video_gr couldn't map video memory");
	return *ptr;
}

//initializes video graphics mode
void *vg_init(unsigned short mode)
{
	/* Allow memory mapping */

	vbe_mode_info_t info;
	if(vbe_get_mode_info(mode, &info))
		return NULL;

	unsigned long vram_base ;/*= VRAM_PHYS_ADDR;*/
	unsigned long vram_size;
	h_res = info.XResolution;
	v_res = info.YResolution;
	bits_per_pixel = (unsigned) info.BitsPerPixel;
	bytes_per_pixel = bits_per_pixel/8;
	vram_phys_addr =  (void*)info.PhysBasePtr;
	vram_base = (unsigned long) info.PhysBasePtr;
	vram_size=(h_res * v_res * bits_per_pixel )/8;
	secondary_buf = malloc(vram_size);
	if(secondary_buf== NULL)
	{
		printf("Not enough memory for secondary buffer\n");
		return NULL;
	}
	map_virtual_memory(vram_base, vram_size, (void**)&video_mem);
	if(set_video_mode(mode))
		return NULL;
	return video_mem;
}
int write_pixelxy(char* buf,unsigned short x, unsigned short y, unsigned long color)
{
	if(x < 0 || x >=h_res || y < 0 || y >=v_res)
		return 1;
	void* pixel = (void*) (buf + y * h_res * bytes_per_pixel + x * bytes_per_pixel);
	memcpy(pixel, (void*)&color, bytes_per_pixel);
	//*pixel = (char) color;
	return 0;
}
int write_pixel(char* buf,coord_t c, unsigned long color)
{
	if(c.x < 0 || c.x >=h_res || c.y < 0 || c.y >=v_res)
		return 1;
	void* pixel = (void*) (buf + c.y * h_res * bytes_per_pixel + c.x * bytes_per_pixel);
	memcpy(pixel, (void*)&color, bytes_per_pixel);
	return 0;
}

unsigned long read_pixel(char * buf, int x, int y)
{
	if(x < 0 || x >=h_res || y < 0 || y >=v_res)
		return -1;
	void* pixel = (void*)(buf + y * h_res * bytes_per_pixel + x * bytes_per_pixel);
	unsigned long out = 0;
	memcpy((void*) &out, pixel, bytes_per_pixel);
	return out;
}
int draw_square(char* buf, unsigned short x, unsigned short y, unsigned short size, unsigned long color)
{
	coord_t pos;
	pos.x = x;
	pos.y = y;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < size; i++)
	{
		for(; j < size; j++)
		{
			write_pixel(buf,pos,color);
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = x;
	}
	return 0;
}
typedef struct
{
	coord_t posi;
	coord_t posf;
}line_t;

void initialize_line_t(line_t* l, unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf)
{
	l->posi.x = (int) xi;
	l->posi.y = (int) yi;
	l->posf.x = (int) xf;
	l->posf.y = (int) yf;
}

coord_t doubles_to_coord_t(double x, double y)
{
	coord_t out;
	out.x = (int) (x+.5);
	out.y = (int) (y+.5);
	return out;
}
void swap_i_f(line_t * l)
{
	coord_t temp = l->posi;
	l->posi = l->posf;
	l->posf = temp;
}
int draw_line(char * buf, unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf, unsigned long color)
{
	line_t line;
	initialize_line_t(&line, xi, yi, xf, yf);
	double m ;
	if(line.posf.x-line.posi.x == 0)//avoid division by 0
		if(line.posf.y-line.posi.y > 0)
			m = INT_MAX;
		else m = -INT_MAX;
	else m = (double)(line.posf.y-line.posi.y)/(line.posf.x-line.posi.x);
	double posx = (double) line.posi.x;
	double posy = (double) line.posi.y;
	int step;
	if(abs(m) <= 1){
		step = ((line.posf.x -line.posi.x)>0)? 1:-1;
		for(; abs(posx - line.posf.x) > 0; posx+=step)
		{
			write_pixel(buf,doubles_to_coord_t(posx, posy), color);
			posy += m*step;
		}
		write_pixel(buf,doubles_to_coord_t(posx, posy), color);
	}
	else
	{
		step = ((line.posf.y -line.posi.y)>0)? 1:-1;
		for(;abs(posy - line.posf.y) > 0; posy+=step)
		{
			write_pixel(buf,doubles_to_coord_t(posx, posy), color);
			posx += (1/m)*step;
		}
		write_pixel(buf,doubles_to_coord_t(posx, posy), color);
	}
	return 0;
}


int draw_hor_line(char* buf, unsigned short x, unsigned short y, unsigned short length, unsigned long color)
{
	int pos = 0;
	int count =0;
	for(;count < length; count++)
	{
		write_pixelxy(buf,x+pos,y, color);
		if(pos > 0)
			pos = -pos;
		else pos= (-pos+1);
	}
	return 0;
}

int draw_circle(char * buf, unsigned short x, unsigned short y, unsigned short radius, unsigned long color)
{
	int lines=-radius;
	int pos = 0;
	double length;
	for(; lines < radius;lines++)
	{
		length = (double) 2*sqrt(pow((double)radius,2) - pow((double)lines,2));
		draw_hor_line(buf,x,y+lines,(int)length,color);
	}
	return 0;
}

int draw_xpm(char* buf, int xi, int yi, char* xpm[])
{
	int width, height;
	char* pix = NULL;
	if((pix = (char*) read_xpm(xpm, &width, &height)) == NULL)
		return 1;
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < height; i++)
	{
		for(; j < width; j++)
		{
			if(*pix != 0)
			write_pixel(buf,pos,*pix);
			pix++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}

int draw_pixmap(char* buf, int xi, int yi, int height, int width, char* pix)
{
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < height; i++)
	{
		for(; j < width; j++)
		{
			if(*pix != 0)
			write_pixel(buf,pos,*pix);
			pix++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}

//copy from one buffer to another
int copy_video_buffer(char* video_mem, char* secondary_buffer)//for use in double bufering
{
	memcpy(video_mem, secondary_buffer, v_res*h_res*bits_per_pixel/8);//copy from secondary buffer to video memory
	return 0;
}
//copy from secondary buffer
int commit_to_video_mem(void)
{
	memcpy(video_mem, secondary_buf, v_res*h_res*bits_per_pixel/8);
	return 0;
}
int blank(char* buf)
{
	if(buf == NULL)
		return 1;
	memset(buf,0,bytes_per_pixel*v_res*h_res); // clear buffer
	return 0;
}
int blank_secondary_buf(void)
{
	return blank(secondary_buf);
}
int vg_exit() {

  struct reg86u reg86;

  reg86.u.b.intno = 0x10; /* BIOS video services */

  reg86.u.b.ah = 0x00;    /* Set Video Mode function */
  reg86.u.b.al = 0x03;    /* 80x25 text mode*/

  if( sys_int86(&reg86) != OK ) {
      printf("\tvg_exit(): sys_int86() failed \n");
      return 1;
  } else
      return 0;
}
