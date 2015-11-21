#include <minix/drivers.h>
#include <sys/video.h>
#include <sys/mman.h>

#include <assert.h>

#include "vt_info.h"

#include "video_txt.h"

/* Private global variables */

static char *video_mem;		/* Address to which VRAM is mapped */

static unsigned scr_width;	/* Width of screen in columns */
static unsigned scr_lines;	/* Height of screen in lines */

void vt_fill(char ch, char attr) {
  
  char* vptr = video_mem;
  int i=0;
  for(; i < scr_lines*scr_width; i++)
  {
		  *vptr = ch;
		  vptr++;
		  *vptr = attr;
		  vptr++;
  }
  
}

void vt_blank() {

  vt_fill(' ', 0x0);

}

int vt_print_char(char ch, char attr, int r, int c) {
	if (r >= scr_lines || r < 0)
		return 1;
	if (c >= scr_width || c < 0)
		return 1;
	char* vptr = video_mem;
	vptr+=r* scr_width*2 +2*c;
	*vptr = ch;
	vptr++;
	*vptr= attr;
	vptr++;
	return 0;
}

int vt_print_string(char *str, char attr, int r, int c) {
		if(r > scr_lines - 1 || r < 0)
			  return 1;
		if(c > scr_width - 1 || c < 0)
		  	  return 1;
		char* vptr = video_mem;
		vptr = vptr + (r * scr_width * 2 + c * 2);
		char* max = video_mem + scr_width * scr_lines * 2;
		while(*str != '\0')
		{
			if(vptr >= max)
				return 1;
			*vptr = *str;
			vptr++;
			*vptr = attr;
			vptr++;
			str++;
		}
		return 0;
}

char* int_to_string(int num)
{
	char negative = num<0;
	  int digits = 1;
	  if (negative)
	  {
		  digits++;
		  num = -num;
	  }
	  int tempnum = num;
	  while (tempnum > 10)
	  {
		  tempnum = tempnum / 10;
		  digits++;
	  }
	  char* strtemp = (char*) malloc(digits + 1);
	  char* strptr = strtemp + digits;
	  *strptr = '\0';
	  strptr--;
	  while(digits > 0)
	  {
		  if (digits == 1 && negative)
			  *strptr = '-';
		  else
		  {
			  *strptr = num % 10 + '0';
			  num = num / 10;
		  }
		  digits--;
		  strptr--;
	  }
	  return strtemp;
}

int vt_print_int(int num, char attr, int r, int c) {
  char* strtemp = int_to_string(num);
  vt_print_string(strtemp, attr, r ,c);
  free (strtemp);
  return 0;
}


int vt_draw_rectangle(int width, int height, char attr, int r, int c) {

  char* vptr = video_mem;
  vptr = vptr + (r * scr_width * 2 + c * 2);
  char* max = video_mem + scr_width * scr_lines * 2;
  int lin = 0;
  for(; lin < height; lin++)
  {
	  char* vptrtemp = vptr;
	  int col = 0;
	  for(; col < width; col++)
	  {
		  if(vptr >= max)
			  return 1;
		  vptr++;
		  *vptr = attr;
		  vptr++;
	  }
	  vptr= vptrtemp + scr_width * 2;
  }
  return 0;
}
int vt_draw_frame(int width, int height, char attr, int r, int c)
{
	if(width < 0 || height < 0)
		return -1;
	if(r > scr_lines - 1 || r < 0)
		return 1;
	if(c > scr_width - 1 || c < 0)
   	    return 1;

	  char* vptr = video_mem;
	  vptr = vptr + (r * scr_width * 2 + c * 2);
	  char* max = video_mem + scr_width * scr_lines * 2;
	  int lin = 0;
	  for(; lin < height; lin++)
	  {
		  char* vptrtemp = vptr;
		  int col = 0;
		  if(lin ==0 || lin == height-1)
		  for(; col < width; col++)
		  {
			  if(vptr >= max)
				  return 1;
			  vptr++;
			  *vptr = attr;
			  vptr++;
		  }
		  else{
			  if(vptr >= max)
				  return 1;
			 vptr++;
			 *vptr = attr;
			  vptr++;

			  vptr = vptr + (width-2)*2;

			  if(vptr >= max)
				  return 1;
			  vptr++;
			 *vptr = attr;
			  vptr++;
		  }
		  vptr= vptrtemp + scr_width * 2;
	  }
	  return 0;
}
/*
 * THIS FUNCTION IS FINALIZED, do NOT touch it
 */

char *vt_init(vt_info_t *vi_p) {

  int r;
  struct mem_range mr;

  /* Allow memory mapping */

  mr.mr_base = (phys_bytes)(vi_p->vram_base);
  mr.mr_limit = mr.mr_base + vi_p->vram_size;

  if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
	  panic("video_txt: sys_privctl (ADD_MEM) failed: %d\n", r);

  /* Map memory */

  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vi_p->vram_size);

  if(video_mem == MAP_FAILED)
	  panic("video_txt couldn't map video memory");

  /* Save text mode resolution */

  scr_lines = vi_p->scr_lines;
  scr_width = vi_p->scr_width;

  return video_mem;
}
