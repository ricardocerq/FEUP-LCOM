#ifndef PROGRAM_H
#define PROGRAM_H
#include "game.h"
/**
 * @defgroup program program
 * @{
 *
 * Initialize resources used by program, global variables
 */


static ppm_t* background;
static char* video_mem;
static char* secondary_buf;
static ppm_t* cell_ppms[CELL_PPM_COUNT];
static char cell_ppms_filled[CELL_PPM_COUNT];
static ppm_t*  original;
static cursor_t cursor;
static mouse_state_t mouse;
static mouse_state_t previous_mouse;
static char* date_str;
static int mouse_irq_set;
static int kbd_irq_set;
static int timer_irq_set;
static int rtc_irq_set;
static int com1_irq_set;
static int com2_irq_set;
static char * playername;
static unsigned long programinitialtime;
static char* player2name;
static unsigned long player2time;
static role_t role;
static char* dir;
static char* dir_temp;

#define PROGRAM_SIGNATURE 0xC5
#define MAX_PLAYER_NAME_LENGTH 15
#define SET_COMMUNICATION_PARAM() ser_set(1,8,1,0,STD_RATE_VALUE_8)
#define PROGRAM_VIDEO_MODE 0x117
#define DIR_MAX 128

///resets video mode
void reset_video_mode();

///@brief initializes IO devices, subscription of interrupts, set video mode
///@return zero on sucess, non-zero otherwise
int io_init();

///@brief unsubscription of IO interrupts, exit video_mem
///@return zero on sucess, non-zero otherwise
int io_exit();

///@brief initializes rtc data
///@return zero on sucess, non-zero otherwise
int rtc_data_init();

///@brief initializes program
///@return zero on sucess, non-zero otherwise
int initialize_program();

///@brief free resources used by program
void free_program_resources();

///@brief programs main
///@param argc number of arguments
///@param argv arguments
///@return success
int main(int argc, char **argv);

///@brief access background
///@return background
ppm_t* get_background();

///@brief access video_mem
///@return video_mem
char* get_program_video_mem();

///@brief access secondary_buf
///@return secondary_buf
char* get_program_secondary_buf();

///@brief access mouse_irq_set
///@return mouse_irq_set
int get_mouse_irq_set();

///@brief access kbd_irq_set
///@return kbd_irq_set
int get_kbd_irq_set();

///@brief access timer_irq_set
///@return timer_irq_set
int get_timer_irq_set();

///@brief access rtc_irq_set
///@return rtc_irq_set
int get_rtc_irq_set();

///@brief access com1_irq_set
///@return com1_irq_set
int get_com1_irq_set();

///@brief modify com1_irq_set
///@param new_com1 new value for com1_irq_set
///@return newcom1_irq_set
int set_com1_irq_set(int new_com1);

///@brief access com2_irq_set
///@return com2_irq_set
int get_com2_irq_set();

///@brief access com2_irq_set
///@param new_com2 new value for com2_irq_set
///@return com2_irq_set
int set_com2_irq_set(int new_com2);

///@brief access playername
///@return playername
char* get_program_playername();

///@brief access programinitialtime
///@return programinitialtime
unsigned long* get_programinitialtime_ptr();

///@brief modify playername
///@param new_p_n new value for playername
///@return new value for playername
char* set_program_playername(char* new_p_n);

///@brief access playername2
///@return playername2
char* get_program_playername2();

///@brief modify playername2
///@param new_p_n new value for playername2
///@return playername2
char* set_program_player2name(char* new_p_n);

///@brief access player2time
///@return player2time
unsigned long get_player2time();

///@brief modify player2time
///@param new_v new value for player2time
///@return new player2time
unsigned long set_player2time(unsigned long new_v);

///@brief modify role
///@param new_role new value for role
///@return new role
role_t set_role(role_t new_role);

///@brief access role
///@return role
role_t get_role();

///@brief access date string
///@return date string
char** get_date_str_ptr();

///@brief initializes a single cell's pixmap
///@param index of cell
void initialize_single_cell_pixmap(int index);

///@brief Allocates space for all the pixmaps
void allocate_ppm_space();

///@brief Gets the pixmap for a cell of a certain size
///@param radius of cell
///@return pixmap of cell
ppm_t* get_cell_pixmap(float radius);

///@brief free space used by the images of the cells
void free_cell_ppms();

///@brief access current cursor
///@return current cursor
cursor_t* get_cursor();

///@brief access current mouse state
///@return current mouse state
mouse_state_t* get_mouse();

///@brief access previous mouse state
///@return previous mouse state
mouse_state_t* get_previous_mouse();

///@brief access absolute path of file
///@param file file to access
///@return absolute path of file
char* get_dir(char * file);

/** @} end of program */

#endif
