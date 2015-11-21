#include "font.h"
#include "program.h"
#include "video_gr.h"
#include <stdbool.h>
#include <stdlib.h>

static ppm_t* chars1[NUM_FONTSIZES][FONT_END-FONT_START+1];

ppm_t* get_char_ppm(char character, int size)
{
	if(character < FONT_START || character > FONT_END)
		return NULL;
	size--;
	if(size < 0)
		size = 0;
	if(size > NUM_FONTSIZES - 1)
		size = NUM_FONTSIZES - 1;
	return chars1[size][character - FONT_START];
}
int get_char_width(char character, int size)
{
	ppm_t* c = get_char_ppm(character, size);
	if(c == NULL)
		return get_space_width(size);
	else
		{
		if(character == 'r')
			return c->width * .75;
			return c->width;
		}
}

int get_string_width(char* str, int size)
{
	size_t i = 0;
	int sum = 0;
	for(; str[i] != '0'; i++)
	{
		sum += get_char_width(str[i], size) + get_char_sep(size);
	}
	sum -= get_char_sep(size);
	return sum;
}

int get_space_width(int size)
{
	size--;
	if(size < 0)
		size = 0;
	if(size > NUM_FONTSIZES - 1)
		size = NUM_FONTSIZES - 1;
	return SPACE_LENGTH / (NUM_FONTSIZES - size) +.5;
}


int get_line_space(int size)
{
	ppm_t* c = get_char_ppm('!', size);
	if(c == NULL)
		return 0;
	else return c->height;
}
int get_char_sep(int size)
{
	size--;
	if(size < 0)
		size = 0;
	if(size > NUM_FONTSIZES - 1)
		size = NUM_FONTSIZES - 1;
	return CHAR_SEP / (NUM_FONTSIZES - size) +.5;
}

int initialize_single_char(size_t index, ppm_t* font)
{
	if(index > FONT_END)
		return 1;
	int xi, xf, yi, yf, posx, posy;
	ppm_t* current_char = (ppm_t*) malloc(sizeof(ppm_t));
	current_char->height = INITIAL_FONT_SIZE;
	xi = INITIAL_FONT_SIZE * (index % CHARS_PER_COL);
	xf = xi + INITIAL_FONT_SIZE;
	yi = INITIAL_FONT_SIZE * (index / CHARS_PER_COL);
	yf = yi + INITIAL_FONT_SIZE;
	bool continue_condition = true;
	unsigned short current_color;
	unsigned short current_alpha;
	for(; xi < xf && continue_condition; xi++)
	{
		for(posy = yi; posy < yf && continue_condition; posy++)
		{
			read_pixelxy_ppm(font->color, xi, posy,&current_color, font->height, font->width);
			if(current_color != 0)
				continue_condition = false;
		}
	}
	xi -= 2;//adjust
	continue_condition=true;
	for(; xf > xi && continue_condition; xf--)
	{
		for(posy = yi; posy < yf && continue_condition; posy++)
		{
			if(read_pixelxy_ppm(font->color, xf, posy,&current_color, font->height, font->width))
				continue;
			if(current_color != 0)
				continue_condition = false;
		}
	}
	xf+=3;//adjust
	current_char->width = xf-xi;
	current_char->color = (unsigned short*) malloc(sizeof(unsigned short) * current_char->width * current_char->height);
	current_char->alpha = (unsigned short*) malloc(sizeof(unsigned short) * current_char->width * current_char->height);
	for(posx = xi; posx < xf; posx++)
	{
		for(posy = yi; posy < yf; posy++)
		{
			read_pixelxy_ppm(font->color, posx, posy,&current_color, font->height, font->width);
			read_pixelxy_ppm(font->alpha, posx, posy,&current_alpha, font->height, font->width);
			write_pixelxy_ppm(current_char->color, posx-xi,posy-yi, current_color,  current_char->height, current_char->width);
			write_pixelxy_ppm(current_char->alpha, posx-xi,posy-yi, current_alpha,  current_char->height, current_char->width);
		}
	}
	chars1[NUM_FONTSIZES-1][index] = current_char;
	return 0;
}

int initialize_font()
{
	ppm_t* fonts = (ppm_t*) read_ppm(get_dir("/ppms/font.ppm"),get_dir("/ppms/font.ppm"));
	if(fonts == NULL) return 1;
	size_t i,n;
	int xi, xf, yi, yf;
	for(i = 0; i <= FONT_END - FONT_START; i++)
	{
		initialize_single_char(i, fonts);
	}
	for(n = 0; n < NUM_FONTSIZES-1; n++)
	{
		for(i = 0; i <= FONT_END - FONT_START; i++)
		{
			chars1[n][i] = (ppm_t *)reduce_float(chars1[NUM_FONTSIZES-1][i], (NUM_FONTSIZES - n) );
		}
	}
	delete_ppm_t(fonts);
	return 0;
}

int free_fonts()
{
	size_t i, n;
	for(n = 0; n < NUM_FONTSIZES; n++)
	{
		for(i = 0; i <= FONT_END - FONT_START; i++)
		{
			delete_ppm_t(chars1[n][i]);
		}
	}
	return 0;
}

