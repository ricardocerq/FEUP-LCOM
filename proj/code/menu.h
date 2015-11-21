#ifndef MENU_H
#define MENU_H

#include "ppm.h"
#include "utils.h"
#include "ps2mouse.h"
#include <stdbool.h>

/**
 * @defgroup menu menu
 * @{
 *
 * implementation of menus
 */

#define NUM_MAIN_MENU_BUTTONS 4
#define MAIN_MENU_BUTTON_SIZE 8
#define NUM_LOBBY_BUTTONS 2
#define LOBBY_MENU_BUTTON_SIZE 8
#define FADE_RATE 10
#define FADE_MAX 127
#define IM_MESSAGE_MAX 40

///@brief possible lobby states
typedef enum {PLAYER1_READY, PLAYER2_READY, BOTH_READY, NOT_READY }lobby_state_t;

typedef struct
{
	coord_t pos;
	int height;
	int width;
	char* text;
	int size;
	int (*action)();
	float trans;
}button_t;

///@brief initialize a button
///@param pos position of button
///@param text text of button
///@param action function to call when button is clicked
///@param size size of button
///@param trans transparency of button
///@return initialized button
button_t* initialize_button_t (coord_t pos,char * text, int(*action)(), int size, float trans);

///@brief function to be executed when 'Quit' button is pressed
///@return success of operation
int exit_game();

///@brief initialize the main menu buttons
void initialize_main_menu_buttons();

///@brief function to be executed when 'Back' button is pressed
///@return success of operation
int back_button();

///@brief function to be executed when 'Play' button is pressed
///@return success of operation
int play_button();

///@brief initializes the lobby's buttons
void initialize_lobby_buttons();

///@brief checks if mouse is hovering a certain button
///@param b button to check
///@return true if mouse is hovering
bool mouse_is_hovering(button_t * b);

///@brief draw a button, centered
///@param b button to draw
///@return success of operation
int draw_button_t_centered(button_t* b);

///@brief
///@param b
///@return success of operation
int draw_name_button_t(button_t* b);

///@brief
///@param previous_mouse
///@param mouse
///@return success of operation
int main_menu_mouse_event(mouse_state_t* previous_mouse,mouse_state_t* mouse);

///@brief dispatcher
///@param previous_mouse
///@param mouse
///@return success of operation
int lobby_mouse_event(mouse_state_t* previous_mouse,mouse_state_t* mouse);

///@brief renders the main menu
///@return success of operation
int main_menu_render();

///@brief main menu function
///@return success of operation
int main_menu();

///@brief render get player name screen
///@return success of operation
int get_player_name_render();

///@brief
///@return success of operation
int get_player_name();

///@brief transmission in lobby
void establish_connection_transmission();

///@brief reception in lobby
void establish_connection_reception();

///@brief renders multiplayer lobby
///@return success of operation
int mp_lobby_render();

///@brief multiplayer lobby
///@return success of operation
int mp_lobby();

///@brief every second send signal to establish connection, determined by rtc alarm int
///@return success of combination
int on_rtc_alarm_int_transmission();

/** @} end of rtc */

#endif
