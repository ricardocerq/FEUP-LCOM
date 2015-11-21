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
#include "i8254.h"
#include "ppm.h"
#include "font.h"
#include "game.h"
#include <math.h>
#include <stdlib.h>

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
#define max(a,b) ((a>b)? a:b)
/* Private global variables */

static char *video_mem=NULL;		/* Process address to which VRAM is mapped */
static char *secondary_buf=NULL;			//double buffer
static unsigned h_res=0;		/* Horizontal screen resolution in pixels */
static unsigned v_res=0;		/* Vertical screen resolution in pixels */
static unsigned bits_per_pixel=0; /* Number of VRAM bits per pixel */
static unsigned bytes_per_pixel=0;
static void* vram_phys_addr=NULL;

char* get_video_mem(void)
{
	return video_mem;
}
char* get_secondary_buf(void)
{
	return secondary_buf;
}
size_t get_vram_size(void)
{
	return bytes_per_pixel * h_res * v_res;
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
	//*pixel = (char) color;
	return 0;
}

int read_pixelxy(char * buf, int x, int y, unsigned long * out)
{
	if(x < 0 || x >=h_res || y < 0 || y >=v_res)
		return 1;
	void* pixel = (void*)(buf + y * h_res * bytes_per_pixel + x * bytes_per_pixel);
	memcpy((void*) out, pixel, bytes_per_pixel);
	return 0;
}
int read_pixel(char * buf, coord_t c, unsigned long* out)
{
	if(c.x < 0 || c.x >=h_res || c.y < 0 || c.y >=v_res)
		return 1;
	void* pixel = (void*)(buf + c.y * h_res * bytes_per_pixel + c.x * bytes_per_pixel);
	memcpy((void*) out, pixel, bytes_per_pixel);
	return 0;
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

void initialize_line_t(line_t* l, unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf)
{
	l->posi.x = (int) xi;
	l->posi.y = (int) yi;
	l->posf.x = (int) xf;
	l->posf.y = (int) yf;
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
	if(line.posf.x-line.posi.x == 0)
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
	}
	else
	{
		step = ((line.posf.y -line.posi.y)>0)? 1:-1;
		for(;abs(posy - line.posf.y) > 0; posy+=step)
		{
			write_pixel(buf,doubles_to_coord_t(posx, posy), color);
			posx += (1/m)*step;
		}
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

int draw_ppm(char* buf, int xi, int yi, ppm_t* ppm)
{
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short* color;
	if(ppm->alpha == NULL)
		color = ppm->color;
	else
		color = ppm->color;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < ppm->height; i++)
	{
		for(; j < ppm->width; j++)
		{
			write_pixel(buf,pos,*color);
			color++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}
int draw_ppm_alpha(char* buf,int xi, int yi, ppm_t* ppm)
{
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short* color = ppm->color;
	unsigned short* alpha = ppm->alpha;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < ppm->height; i++)
	{
		for(; j < ppm->width; j++)
		{
			unsigned long pixelcolor;
			if(*alpha != 0)
			{
				if (!read_pixel(buf, pos, &pixelcolor))
				{
					//double transparency = (double)(*alpha)/MAX_ALPHA;
					pixelcolor = RGB((GET_R(pixelcolor)*(MAX_ALPHA-*alpha)+GET_R(*color)*(*alpha))>> 7,
							(GET_G(pixelcolor)*(MAX_ALPHA-*alpha)+GET_G(*color)*(*alpha))>> 7,
							(GET_B(pixelcolor)*(MAX_ALPHA-*alpha)+GET_B(*color)*(*alpha))>> 7);
					write_pixel(buf,pos,(short)pixelcolor);
				}
				else write_pixel(buf,pos,*color);
			}
			color++;
			alpha++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}
int draw_char(char* buf,char character,int xi, int yi, float transparency, unsigned short newcolor, int size)
{
	ppm_t* ppm = (ppm_t*)get_char_ppm(character, size);
	if(ppm == NULL)
		return 1;
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short* color = ppm->color;
	unsigned short* alpha = ppm->alpha;
	unsigned short i = 0;
	unsigned short j = 0;
	unsigned short currentalpha;
	for(; i < ppm->height; i++)
	{
		for(; j < ppm->width; j++)
		{
			unsigned long pixelcolor;
			if(*alpha != 0)
			{
				if (!read_pixel(buf, pos, &pixelcolor))
				{
					currentalpha = *alpha * transparency;
					pixelcolor = RGB((GET_R(pixelcolor)*(MAX_ALPHA-currentalpha)+GET_R(newcolor)*(currentalpha))>> 7,
							(GET_G(pixelcolor)*(MAX_ALPHA-currentalpha)+GET_G(newcolor)*(currentalpha))>> 7,
							(GET_B(pixelcolor)*(MAX_ALPHA-currentalpha)+GET_B(newcolor)*(currentalpha))>> 7);
					write_pixel(buf,pos,(short)pixelcolor);
				}
				else write_pixel(buf,pos,*color);
			}
			color++;
			alpha++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}
int draw_cursor(char* buf, cursor_t* cursor)
{
	ppm_t* ppm = cursor->pixmap;
	if(ppm == NULL)
		return 1;
	coord_t pos;
	pos.x = cursor->pos.x - cursor->pixmap->width/2;
	pos.y = cursor->pos.y - cursor->pixmap->height/2;
	unsigned short* color = ppm->color;
	unsigned short* alpha = ppm->alpha;
	unsigned short i = 0;
	unsigned short j = 0;
	unsigned short currentalpha;
	float transparency = 0.9;
	unsigned short newcolor = RGB(MAX_RED, MAX_GREEN, MAX_BLUE);
	for(; i < ppm->height; i++)
	{
		for(; j < ppm->width; j++)
		{
			unsigned long pixelcolor;
			if(*alpha != 0)
			{
				if (!read_pixel(buf, pos, &pixelcolor))
				{
					currentalpha = *alpha * transparency;
					//double transparency = (double)(*alpha)/MAX_ALPHA;
					pixelcolor = RGB((GET_R(pixelcolor)*(MAX_ALPHA-currentalpha)+GET_R(newcolor)*(currentalpha))>> 7,
							(GET_G(pixelcolor)*(MAX_ALPHA-currentalpha)+GET_G(newcolor)*(currentalpha))>> 7,
							(GET_B(pixelcolor)*(MAX_ALPHA-currentalpha)+GET_B(newcolor)*(currentalpha))>> 7);
					write_pixel(buf,pos,(short)pixelcolor);
				}
				else write_pixel(buf,pos,*color);
			}
			color++;
			alpha++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x =cursor->pos.x - cursor->pixmap->width/2;
	}
	return 0;
}
int printgr(char* buf, char* str, int x, int y, float trans, unsigned short color, int size)
{
	size_t i;
	int savedx = x;
	for(i=0; str[i] != '\0'; ++i)
	{
		if(str[i] == '\n')
		{
				x = savedx;
				y += get_line_space(size);
		}
		if(str[i] < FONT_START || str[i] > FONT_END)
			x+=get_space_width(size);
		else{
			if(draw_char(buf,str[i],x,y,trans,color,size))
				continue;
			x+= get_char_width(str[i], size) + get_char_sep(size);
		}
	}
	return 0;
}
int printgr_shade(char* buf, char* str, int x, int y, float trans_f, unsigned short color_f, int xoffset, int yoffset, float shadetrans_f, int size)
{
	if(printgr(buf, str, x+xoffset, y+yoffset, shadetrans_f,0, size))
		return 1;
	return printgr(buf, str,x,y, trans_f,color_f, size);
}
int printgr_shade_centered(char* buf, char* str, int y, float trans_f, unsigned short color_f, int xoffset, int yoffset, float shadetrans_f, int size)
{
	size_t i = 0;
	int width = 0;
	for(; str[i] != '\0'; i++)
	{
		width += get_char_width(str[i], size) + get_char_sep(size);
	}
	width -= get_char_sep(size);
	int x = h_res / 2 - width / 2;
	if(printgr(buf, str, x+xoffset, y+yoffset, shadetrans_f,0, size))
		return 1;
	return printgr(buf, str,x,y, trans_f,color_f, size);
}
int draw_ppm_alpha_player(char* buf,int xi, int yi, ppm_t* ppm)
{
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short* color = ppm->color;
	unsigned short* alpha = ppm->alpha;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < ppm->height; i++)
	{
		for(; j < ppm->width; j++)
		{
			unsigned long pixelcolor;
			if (!read_pixel(buf, pos, &pixelcolor))
			{
			//double transparency = (double)(*alpha)/MAX_ALPHA;
			pixelcolor = RGB((GET_R(pixelcolor)*(MAX_ALPHA-*alpha)+GET_R(*color)*(*alpha))>> 7,
							 (GET_G(pixelcolor)*(MAX_ALPHA-*alpha)+((GET_G(*color) + 10> MAX_GREEN)? MAX_GREEN : (GET_G(*color) + 10))*(*alpha))>> 7,
							 (GET_B(pixelcolor)*(MAX_ALPHA-*alpha)+GET_B(*color)*(*alpha))>> 7);
			write_pixel(buf,pos,(short)pixelcolor);
			}
			else write_pixel(buf,pos,*color);
			color++;
			alpha++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}
int draw_ppm_alpha_enemy(char* buf,int xi, int yi, ppm_t* ppm, unsigned short addred)
{
	coord_t pos;
	pos.x = xi;
	pos.y = yi;
	unsigned short* color = ppm->color;
	unsigned short* alpha = ppm->alpha;
	unsigned short i = 0;
	unsigned short j = 0;
	addred = addred << 2;
	if(addred > 0)
		addred += 1;
	if(addred > 18)
		addred = 18;
	for(; i < ppm->height; i++)
	{
		for(; j < ppm->width; j++)
		{
			unsigned long pixelcolor;
			if (!read_pixel(buf, pos, &pixelcolor))
			{
			pixelcolor = RGB((GET_R(pixelcolor)*(MAX_ALPHA-*alpha)+((GET_R(*color) + addred> MAX_RED)? MAX_RED : (GET_R(*color) + addred))*(*alpha))>> 7,
					         (GET_G(pixelcolor)*(MAX_ALPHA-*alpha)+GET_G(*color)*(*alpha))>> 7,
							 (GET_B(pixelcolor)*(MAX_ALPHA-*alpha)+GET_B(*color)*(*alpha))>> 7);
			write_pixel(buf,pos,(short)pixelcolor);
			}
			else write_pixel(buf,pos,*color);
			color++;
			alpha++;
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = xi;
	}
	return 0;
}

int darken_buf(char* buf,unsigned short dark)
{
	if(dark > 127)
		dark = 127;
	size_t i = 0;
	size_t lim = v_res*h_res;
	unsigned long current_color = 0;
	for(; i < lim; i++)
	{
		current_color = *(unsigned short*)buf;
		*(unsigned short*)buf = RGB((GET_R(current_color)*(MAX_ALPHA-dark))>> 7,
									(GET_G(current_color)*(MAX_ALPHA-dark))>> 7,
									(GET_B(current_color)*(MAX_ALPHA-dark))>> 7);
		buf+=2;
	}
	return 0;
}
int copy_video_buffer(char* video_mem, char* secondary_buffer)//fouruse in double bufering
{
	memcpy(video_mem, secondary_buffer, v_res*h_res*bits_per_pixel/8);//copy from secondary buffer to video memory
	return 0;
}
int commit_to_video_mem(void)
{
	memcpy(video_mem, secondary_buf, v_res*h_res*bits_per_pixel/8);
	return 0;
}
int blank(char* buf)
{
	if(buf == NULL)
		return 1;
	memset(buf,0,bytes_per_pixel*v_res*h_res); // clear vram
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
