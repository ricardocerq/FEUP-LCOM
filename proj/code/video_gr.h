#ifndef __VIDEO_GR_H
#define __VIDEO_GR_H

#include "ppm.h"
#include "utils.h"
#include "game.h"

/**
 * @defgroup video_gr video_gr
 * @{
 *
 * Functions for outputing data to screen in graphics mode
 */
#define draw_line_coord_t(buf, c1, c2, color) draw_line(buf,c1.x,c1.y,c2.x,c2.y,color)
#define draw_ppm_alpha_centered( buf,xi,yi,ppm) draw_ppm_alpha( buf, xi - (ppm->width)/2, yi - (ppm->height)/2, ppm)
#define draw_ppm_alpha_centered_player( buf,xi,yi,ppm) draw_ppm_alpha_player( buf, xi - (ppm->width)/2, yi - (ppm->height)/2, ppm)
#define draw_ppm_alpha_centered_enemy(buf,xi,yi,ppm, addred) draw_ppm_alpha_enemy( buf,xi - (ppm->width)/2, yi - (ppm->height)/2,ppm,addred)
#define draw_ppm_centered( buf,xi,yi,ppm) draw_ppm( buf, xi - ppm->width/2, yi - ppm->height/2, ppm)
int printgr(char* buf, char* str, int x, int y, float trans, unsigned short color, int size);
int printgr_shade(char* buf, char* str, int x, int y, float trans_f, unsigned short color_f, int xoffset, int yoffset, float shadetrans_f, int size);
int printgr_shade_centered(char* buf, char* str, int y, float trans_f, unsigned short color_f, int xoffset, int yoffset, float shadetrans_f, int size);
char* get_secondary_buf(void);
int darken_buf(char* buf,unsigned short dark);

/**
 * @brief Initializes the video module in graphics mode
 * 
 * Uses the VBE INT 0x10 interface to set the desired
 *  graphics mode, maps VRAM to the process' address space and
 *  initializes static global variables with the resolution of the screen, 
 *  and the number of colors
 * 
 * @param mode 16-bit VBE mode to set
 * @return Virtual address VRAM was mapped to. NULL, upon failure.
 */
void *vg_init(unsigned short mode);

 /**
 * @brief Returns to default Minix 3 text mode (0x03: 25 x 80, 16 colors)
 * 
 * @return 0 upon success, non-zero upon failure
 */
int vg_exit(void);

typedef struct
{
	coord_t posi;
	coord_t posf;
}line_t;


///@return video_mem
char* get_video_mem(void);

///@return secondary_buf
char* get_secondary_buf(void);

///@return h_res
unsigned get_h_res(void);

///@return v_res
unsigned get_v_res(void);

///@return bits_per_pixel
unsigned get_bits_per_pixel(void);

///@return vram_phys_addr
void* get_vram_phys_addr(void);

///@brief change to a specific video mode
///@param mode
///@return zero on success, non-zero otherwise
int set_video_mode(unsigned short mode);


///@brief maps address to virtual memory of process
///@param base base of vram
///@param size of vram
///@param ptr to virtual memory
void* map_virtual_memory(unsigned long base, unsigned long size, void** ptr);

///@brief write a color to a pixel
///@param buf buffer to write
///@param x x position of pixel
///@param y y position of pixel
///@param color color to write
///@return zero on success, non-zero otherwise
int write_pixelxy(char* buf,unsigned short x, unsigned short y, unsigned long color);

///@brief write a color to a pixel
///@param buf buffer to write
///@param c position of pixel
///@param color color to write
///@return zero on success, non-zero otherwise
int write_pixel(char* buf,coord_t c, unsigned long color);

///@brief read a pixel's color
///@param buf buffer to read
///@param x position
///@param y postion
///@param out return value
///@return zero on success, non-zero otherwise
int read_pixelxy(char * buf, int x, int y, unsigned long * out);


///@brief read a pixel's color
///@param buf buffer
///@param c position
///@param out return value
///@return zero on success, non-zero otherwise
int read_pixel(char * buf, coord_t c, unsigned long* out);

///@brief draw a square
///@param buf buffer
///@param x position
///@param y position
///@param size size of square
///@param color color of square
int draw_square(char* buf, unsigned short x, unsigned short y, unsigned short size, unsigned long color);


///@brief initialize line
///@param l line
///@param xi initial x
///@param yi initial y
///@param xf final x
///@param yf final y
void initialize_line_t(line_t* l, unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf);

///@brief swap initial and final points of line
///@param l line
void swap_i_f(line_t * l);


///@brief draw a line
///@param buf buffer
///@param xi initial x
///@param yi initial y
///@param xf final x
///@param yf final y
///@param color color of line
///@return zero on success, non-zero otherwise
int draw_line(char * buf, unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf, unsigned long color);

///@brief draw horizontal line
///@param buf buffer
///@param x position of center
///@param y position of center
///@param length length of line
///@param color color
///@return zero on success, non-zero otherwise
int draw_hor_line(char* buf, unsigned short x, unsigned short y, unsigned short length, unsigned long color);



///@brief draw a circle
///@param buf buffer
///@param x x position
///@param y y position
///@param radius radius of circle
///@param color color
///@return zero on success, non-zero otherwise
int draw_circle(char * buf, unsigned short x, unsigned short y, unsigned short radius, unsigned long color);


///@brief draw a ppm
///@param buf buffer
///@param xi initial x
///@param yi initial y
///@param ppm ppm to draw
///@return zero on success, non-zero otherwise
int draw_ppm(char* buf, int xi, int yi, ppm_t* ppm);


///@brief draw a ppm, taking into account its transparency
///@param buf buffer
///@param xi initial x
///@param yi initial y
///@param ppm ppm to draw
///@return zero on success, non-zero otherwise
int draw_ppm_alpha(char* buf,int xi, int yi, ppm_t* ppm);

///@brief draw a character on the screen
///@param buf buffer
///@param character character to draw
///@param xi initial x
///@param yi initial y
///@param transparency transparency of character
///@param newcolor color of character
///@param size size of character, from 1 to 10
///@return zero on success, non-zero otherwise
int draw_char(char* buf,char character,int xi, int yi, float transparency, unsigned short newcolor, int size);


///@brief draw a cursor on the screen
///@param buf buffer
///@param cursor cursor to draw
///@return zero on success, non-zero otherwise
int draw_cursor(char* buf, cursor_t* cursor);


///@brief print string on a buffer
///@param buf buffer
///@param str string
///@param x initial x
///@param y initial y
///@param trans transparency of string
///@param color color of strings
///@param size size of strings
///@return zero on success, non-zero otherwise
int printgr(char* buf, char* str, int x, int y, float trans, unsigned short color, int size);



///@brief print a string on a buffer, with a shadow behind it
///@param buf buffer
///@param str string
///@param x initial x
///@param y initial y
///@param trans_f transparency of string
///@param color_f color of string
///@param xoffset offset of shadow
///@param yoffset offset of shadow
///@param shadetrans_f transparency of shadow
///@param size size of string
///@return zero on success, non-zero otherwise
int printgr_shade(char* buf, char* str, int x, int y, float trans_f, unsigned short color_f, int xoffset, int yoffset, float shadetrans_f, int size);


///@brief print a string on a buffer, with a shadow behind it, centered
///@param buf buffer
///@param str string
///@param y initial y
///@param trans_f transparency of string
///@param color_f color of string
///@param xoffset offset of shadow
///@param yoffset offset of shadow
///@param shadetrans_f transparency of shadow
///@param size size of string
///@return zero on success, non-zero otherwise
int printgr_shade_centered(char* buf, char* str, int y, float trans_f, unsigned short color_f, int xoffset, int yoffset, float shadetrans_f, int size);


///@brief draw the player's ppm, adds green to each pixel
///@param buf buffer
///@param xi initial x
///@param yi initial y
///@param ppm ppm to draw
///@return zero on success, non-zero otherwise
int draw_ppm_alpha_player(char* buf,int xi, int yi, ppm_t* ppm);


///@brief draw the enemies' ppm, adds red to each pixel
///@param buf buffer
///@param xi initial x
///@param yi initial y
///@param ppm ppm to draw
///@param addred amount of red to add
///@return zero on success, non-zero otherwise
int draw_ppm_alpha_enemy(char* buf,int xi, int yi, ppm_t* ppm, unsigned short addred);


///@brief darken the buffer by a certain amount
///@param buf buffer
///@param dark amount
///@return zero on success, non-zero otherwise
int darken_buf(char* buf,unsigned short dark);


///@brief copy form one buffer to another
///@param video_mem video memory
///@param secondary_buffer secondary buffer
///@return zero on success, non-zero otherwise
int copy_video_buffer(char* video_mem, char* secondary_buffer);


///@brief commit secondary buffer to video memory
///@return zero on success, non-zero otherwise
int commit_to_video_mem(void);

///@brief blank buffer
///@param buf buffer
///@return zero on success, non-zero otherwise
int blank(char* buf);



///@brief blanks secondary buffer
///@return zero on success, non-zero otherwise
int blank_secondary_buf(void);


 /** @} end of video_gr */
 
#endif /* __VIDEO_GR_H */
