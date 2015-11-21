#ifndef FONT_H
#define FONT_H
#include "ppm.h"
#include <stdlib.h>

/**
 * @defgroup font font
 * @{
 *
 * initialization and manipulation of fonts
 */

#define NUM_FONTSIZES 10
#define SPACE_LENGTH 50
#define NEWLINE_SIZE 150
#define CHAR_SEP 15
#define FONT_START 33
#define FONT_END 126
#define INITIAL_FONT_SIZE 200
#define CHARS_PER_COL 10
#define CHARS_PER_LIN 10


///@brief get the pixmap of a character of a certain size
///@param character character, from '!' to '~' else returns ' '
///@param size size of character's pixmap
///@return pixmap of character
ppm_t* get_char_ppm(char character, int size);

///@brief get the width of the pixmap of a character of a certain size
///@param character character, from '!' to '~' else returns ' '
///@param size size of character
///@return width of pixmap of character
int get_char_width(char character, int size);

///@brief get the width of the pixmaps of a string of a certain size
///@param str string
///@param size size of characters
///@return width of the pixmaps of string
int get_string_width(char* str, int size);

///@brief get the width of a space ' ' of a certain size
///@param size size of character
///@return width of space
int get_space_width(int size);

///@brief get the height of a line of a certain size
///@param size size of characters
///@return height of line
int get_line_space(int size);

///@brief get the width of space between characters of a certain size
///@param size size of characters
///@return width of space between characters
int get_char_sep(int size);

///@brief initialize pixmap of character
///@param index index of character in array, '!' is 0
///@param font pixmap containing all letters
///@return success of operation
int initialize_single_char(size_t index, ppm_t* font);

///@brief initialize all pixmaps of characters
///@return success of operation
int initialize_font();

///@brief free space used by fonts
///@return success of operation
int free_fonts();

/** @} end of font */

#endif
