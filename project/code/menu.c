#include "menu.h"
#include "game.h"
#include "video_gr.h"
#include "mouse.h"
#include "ppm.h"
#include "vector.h"
#include "program.h"
#include "kbd.h"
#include "i8042.h"
#include "ser_port.h"
#include "vector.h"
#include "graphics_mode.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>


static bool continue_condition = true;
static bool fade_condition = false;
static bool function_call_condition = false;
static int(*function_to_call)() = NULL;
static int darken = 0;
static button_t* main_menu_buttons[NUM_MAIN_MENU_BUTTONS];
static button_t* lobby_buttons[NUM_LOBBY_BUTTONS];
static lobby_state_t lobby_state = NOT_READY;
static char opponentmessage[MAX_PLAYER_NAME_LENGTH +10];
static vector_t * im_message_log = NULL;
static char current_im[IM_MESSAGE_MAX];
static char transmission_message[IM_MESSAGE_MAX + 4];
static size_t current_im_index = 0;
static size_t draw_im_index = 0;


button_t* initialize_button_t (coord_t pos,char * text, int(*action)(), int size, float trans)
{
	button_t* out = malloc(sizeof(button_t));
	out->text = text;
	out->pos.y = pos.y;
	out->action = action;
	out->height = get_line_space(size);
	size_t i = 0;
	out->width = 0;
	for(; text[i] != '\0'; i++)
	{
		out->width += get_char_width(text[i], size) + get_char_sep(size);
	}
	out->width -= get_char_sep(size);
	out->pos.x = get_h_res()/2 - out->width /2;
	out->size = size;
	out->trans = trans;
	return out;
}

int exit_game()
{
	fade_condition = true;
	return 0;
}

void initialize_main_menu_buttons()
{
	main_menu_buttons[0] = initialize_button_t(to_coord_t(get_h_res()/2, get_v_res() / 2 - 50), "Single Player", sp_game, MAIN_MENU_BUTTON_SIZE, 0.9);
	main_menu_buttons[1] = initialize_button_t(to_coord_t(get_h_res()/2, get_v_res() / 2 +50), "Multiplayer", mp_lobby, MAIN_MENU_BUTTON_SIZE,0.9);
	main_menu_buttons[2] = initialize_button_t(to_coord_t(get_h_res()/2, get_v_res() / 2 + 150), "Quit", exit_game, MAIN_MENU_BUTTON_SIZE, 0.9);
	main_menu_buttons[3] = initialize_button_t(to_coord_t(10,get_v_res()-get_line_space(2)),get_program_playername(), get_player_name, 2, 0.9);
	main_menu_buttons[3]->pos.x = 10;
}

int back_button()
{
	transmit_message(MES_BYE);
	fade_condition = true;
}
int play_button()
{
	if(lobby_state == PLAYER2_READY)
	{
		transmit_message(MES_READY);
		lobby_state = BOTH_READY;
		fade_condition = true;
	}
	else if(lobby_state == PLAYER1_READY)
	{
		transmit_message(MES_UNREADY);
		 lobby_state = NOT_READY;
	}
	else {
		transmit_message(MES_READY);
		lobby_state = PLAYER1_READY;
	}
}
void initialize_lobby_buttons()
{
	lobby_buttons[0] = initialize_button_t(to_coord_t(get_h_res()/2, get_v_res() / 2 - 50), "PLAY", play_button, LOBBY_MENU_BUTTON_SIZE, 0.9);
	lobby_buttons[1] = initialize_button_t(to_coord_t(get_h_res()/2, get_v_res() / 2 +50), "Back", back_button, LOBBY_MENU_BUTTON_SIZE-1, 0.9);
}

bool mouse_is_hovering(button_t * b)
{
	if(get_cursor()->pos.x > b->pos.x
		&& get_cursor()->pos.x < b->pos.x + b->width
		&& get_cursor()->pos.y > b->pos.y
		&& get_cursor()->pos.y < b->pos.y + b->height)
		return true;
	return false;
}

int draw_button_t_centered(button_t* b)
{
	if(mouse_is_hovering(b))
	return printgr_shade_centered(get_program_secondary_buf(),b->text,b->pos.y, b->trans,0xFFFF,5,5, .2, b->size);
	return printgr_shade_centered(get_program_secondary_buf(),b->text,b->pos.y, b->trans-.4,0xFFFF,5,5, .2, b->size);
}
int draw_name_button_t(button_t* b)
{
	if(mouse_is_hovering(b))
	return printgr_shade(get_program_secondary_buf(),get_program_playername(),b->pos.x,b->pos.y, b->trans,0xFFFF,5,5, .2, b->size);
	return printgr_shade(get_program_secondary_buf(),get_program_playername(),b->pos.x,b->pos.y, b->trans-.4,0xFFFF,5,5, .2, b->size);
}

int main_menu_mouse_event(mouse_state_t* previous_mouse,mouse_state_t* mouse)
{
	if(mouse->lb == 1 && previous_mouse->lb == 0)
	{
		size_t i;
		for(i = 0; i < NUM_MAIN_MENU_BUTTONS; i++)
		{
			if(mouse_is_hovering(main_menu_buttons[i]))
				{
					function_to_call = main_menu_buttons[i]->action;
					fade_condition = true;
					return 1;
				}
		}
	}
	return 0;
}

int lobby_mouse_event(mouse_state_t* previous_mouse, mouse_state_t* mouse)
{
	if(mouse->lb == 1 && previous_mouse->lb == 0)
	{
		size_t i;
		for(i = 0; i < NUM_LOBBY_BUTTONS; i++)
		{
			if(mouse_is_hovering(lobby_buttons[i]))
			{
				lobby_buttons[i]->action();
				return 1;
			}
			else function_to_call = NULL;
		}
	}
	return 0;
}

int main_menu_render()
{
	memcpy(get_program_secondary_buf(),get_background()->color,get_vram_size());
	//draw_cursor(get_program_secondary_buf(),cursor);
	draw_cursor(get_program_secondary_buf(),get_cursor());
	size_t i;
	for(i = 0; i < NUM_MAIN_MENU_BUTTONS-1; i++)
	{
		draw_button_t_centered(main_menu_buttons[i]);
	}
	draw_name_button_t(main_menu_buttons[i]);

	printgr_shade_centered(get_program_secondary_buf(),"O",get_v_res()/2-325, 0.75,0xFFFF,5,5, .1, 10);
	printgr_shade(get_program_secondary_buf(),"2",get_h_res()/2+35, get_v_res()/2-215, 0.75,0xFFFF,5,5, .1,9);
	if(*get_date_str_ptr() != NULL)
		printgr_shade_centered(get_program_secondary_buf(),*get_date_str_ptr(),0, 0.75,0xFFFF,5,5, .1, 1);
	if(darken > 0)
		darken_buf(get_program_secondary_buf(),darken);
	if(darken > 0 && !fade_condition)
	{
		darken-= FADE_RATE;
	}
	if(fade_condition)
	{
		if(darken <= 127)
		{
			darken += FADE_RATE;
		}
		else
		{
			if(function_to_call == NULL)
				continue_condition = false;
			else
			{
				fade_condition = false;
				function_to_call();
				if(main_menu_buttons[3] != NULL)
					free (main_menu_buttons[3]);
				main_menu_buttons[3] = initialize_button_t(to_coord_t(10,get_v_res()-get_line_space(2)),get_program_playername(), get_player_name, 2, 0.9);
				main_menu_buttons[3]->pos.x = 10;
				function_to_call = NULL;
			}
		}
	}
	copy_video_buffer(get_program_video_mem(), get_program_secondary_buf());
	return 0;
}

int main_menu()
{
	continue_condition = true;
	fade_condition = false;
	function_to_call = NULL;
	darken = 127;
	int failure = 0;
	initialize_main_menu_buttons();
	int ipc_status;
	message msg;
	int r;
	char * display_name = (char*)malloc(sizeof(char) * strlen("Player name: ")+MAX_PLAYER_NAME_LENGTH);
	strcpy(display_name, "Player name: ");
	display_name = strcat(display_name, get_program_playername());
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
					if (msg.NOTIFY_ARG & get_kbd_irq_set()){  /* subscribed kbd interrupt */
						kbd_int_handler();
					}
					if (msg.NOTIFY_ARG & get_rtc_irq_set()) { /* subscribed timer interrupt */
						if(rtc_ih())
							failure = 1;
						getDateString(get_date_str_ptr());
					}
					if(msg.NOTIFY_ARG & get_timer_irq_set()){
						timer_int_handler();
						main_menu_render();
					}
					if (msg.NOTIFY_ARG & get_mouse_irq_set()) { /* subscribed timer interrupt */
						mouse_int_handler();
				}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {/* received a standard message, not a notification */
				/* no standard messages expected: do nothing */
			}
			if(mouse_is_updated())
			{
				assign_mouse(get_previous_mouse(), get_mouse());
				assign_mouse(get_mouse(), get_mouse_state());
				main_menu_mouse_event(get_previous_mouse(), get_mouse());
				move_cursor(get_cursor(), get_mouse()->coords);
			}
		} while(continue_condition);
	}
	return failure;
}

int get_player_name_render()
{
	memcpy(get_program_secondary_buf(),get_background()->color,get_vram_size());
	printgr_shade_centered(get_program_secondary_buf(),"Please enter your name",get_v_res()/2-200, 0.75,0xFFFF,5,5, .1, 8);
	printgr_shade_centered(get_program_secondary_buf(),get_program_playername(),get_v_res()/2, 0.75,0xFFFF,5,5, .1,7);
	if(*get_date_str_ptr() != NULL)
		printgr_shade_centered(get_program_secondary_buf(),*get_date_str_ptr(),0, 0.75,0xFFFF,5,5, .1, 1);
	if(darken > 0)
		darken_buf(get_program_secondary_buf(),darken);
	if(darken > 0 && !fade_condition)
	{

		darken-= FADE_RATE;
	}
	if(fade_condition)
	{
		if(darken <= 127)
		{

			darken += FADE_RATE;
		}
		else
		{
			continue_condition = false;
		}
	}
	copy_video_buffer(get_program_video_mem(), get_program_secondary_buf());
	return 0;
}

int get_player_name()
{
		continue_condition = true;
		fade_condition = false;
		darken = FADE_MAX;
		int failure = 0;
		int ipc_status;
		message msg;
		int r;
		if(get_program_playername() == NULL)
		{
			if(set_program_playername((char*)malloc(sizeof(char)*MAX_PLAYER_NAME_LENGTH)) ==NULL)return 1;
			memset(get_program_playername(),0,sizeof(char)*MAX_PLAYER_NAME_LENGTH);
		}
		size_t next_char_index = strlen(get_program_playername());
		char last_char = 0;
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
						if (msg.NOTIFY_ARG & get_kbd_irq_set()){  /* subscribed kbd interrupt */
							kbd_int_handler();
							if(get_scancode() == ENTER_MAKE && next_char_index != 0)
							{
								fade_condition = true;
							}
							else
							{
								int action;
								action = process_input(get_scancode(), &last_char);
								if(action == CHAR_RECEIVED)
								{
									//printf("last_char: %c, %d\n", last_char,last_char);
									if(next_char_index < MAX_PLAYER_NAME_LENGTH-1)
									{
										get_program_playername()[next_char_index] = last_char;
										next_char_index++;
									}
								}
								else if(action == BACKSPACE_MAKE)
								{
									if(next_char_index > 0)
										{
											next_char_index--;
											get_program_playername()[next_char_index] = '\0';
										}
								}
							}
						}
						if (msg.NOTIFY_ARG & get_rtc_irq_set()) { /* subscribed timer interrupt */
							if(rtc_ih())
								failure = 1;
							getDateString(get_date_str_ptr());
						}
						if(msg.NOTIFY_ARG & get_timer_irq_set()){
							timer_int_handler();
							get_player_name_render();
						}
						if (msg.NOTIFY_ARG & get_mouse_irq_set()) { /* subscribed timer interrupt */
							mouse_int_handler();
						}
						break;
					default:
						break; /* no other notifications expected: do nothing */
					}
				} else {/* received a standard message, not a notification */
					/* no standard messages expected: do nothing */
				}

			} while(continue_condition);
		}
		get_program_playername()[next_char_index] = '\0';
		continue_condition = true;
				fade_condition = false;
				darken = FADE_MAX;
		return 0;
}

int on_rtc_alarm_int_transmission()
{
	if(get_role() == CLIENT || get_role() == SERVER || get_role() == FAILED)
			return 0;
	if(get_role() == SEARCHING || get_role() == CONNECTING)
	{
		message_type_t message;
		if(get_role() == SEARCHING)
			message = MES_HELLO;
		else if (get_role() == CONNECTING)
			message = MES_ACK;
		transmit_message(message);
	}
	return 0;
}

void establish_connection_transmission()
{
	if(get_role() == CLIENT || get_role() == SERVER || get_role() == FAILED)
		return;
	if(get_ticks() % 60 == 0)
	{
		if(get_role() == SEARCHING || get_role() == CONNECTING)
		{
			message_type_t message;
			if(get_role() == SEARCHING)
				message = MES_HELLO;
			else if (get_role() == CONNECTING)
				message = MES_ACK;
			transmit_message(message);
		}
		else
			if(get_role() == CONNECTED)
			{}
	}
}

void establish_connection_reception()
{
	if(get_role() == SEARCHING || get_role() == CONNECTING)
	{
		message_type_t received = 0;
		signal_type_t received_signal_type;
		size_t received_size = 0;
		if(receive_protocol(&received,&received_signal_type, &received_size) == 0)
		{
			if(received == MES_HELLO && get_role()== SEARCHING)
			{
				transmit_message(MES_ACK);
				set_role(CONNECTING);
			}
			else if(received == MES_ACK && (get_role() == CONNECTING||get_role() == SEARCHING))
			{
				transmit_message(MES_ACK);
				set_role(CONNECTED);
				transmit_protocol(get_program_playername(), TYPE_STRING, strlen(get_program_playername()));
				transmit_protocol(get_programinitialtime_ptr(), TYPE_INT, sizeof(int));
			}
		}

	}
	else if(get_role() == CONNECTED)
	{
		message_type_t received = 0;
		signal_type_t received_signal_type;
		size_t received_size = 0;

		if(receive_protocol(get_program_playername2(),&received_signal_type, &received_size) == 0)
		{
			if(received_signal_type == TYPE_STRING)
			{
				strcat(opponentmessage, get_program_playername2());
				if(strcmp(get_program_playername(), get_program_playername2()) == 0)
					set_role(FAILED);
				else if(strcmp(get_program_playername(), get_program_playername2()) < 0)
					{
						set_role(SERVER);
					}
				else set_role(CLIENT);
			}
		}
	}
	else {
		char received[100];
		memset(received, 0, 100);
		signal_type_t received_signal_type;
		size_t received_size = 0;
		if(receive_protocol(received,&received_signal_type, &received_size) == 0)
		{
			if(*(unsigned long*)received == MES_BYE)
			{
				set_role(SEARCHING);
				memset(get_program_playername2(),0,sizeof(char)*MAX_PLAYER_NAME_LENGTH+1);
				if(im_message_log != NULL)
				{

					size_t del_i;
					for(del_i = 0 ; del_i < size(im_message_log); del_i++)
					{
						free(*(char**)at(im_message_log, del_i));
					}

					clear(im_message_log);
				}
			}
			else if(received_signal_type == TYPE_INT)
			{
				set_player2time( *(unsigned long*)received);
				if(get_role() == SERVER)
					srand(*get_programinitialtime_ptr());
				else srand(get_player2time());
			}
			else if(*(unsigned long*)received == MES_READY)
			{
				if(lobby_state == NOT_READY)
					lobby_state = PLAYER2_READY;
				else if(lobby_state == PLAYER1_READY)
				{
					lobby_state = BOTH_READY;
					fade_condition = true;
				}
			}
			else if(*(unsigned long*)received == MES_UNREADY)
			{
				if(lobby_state == PLAYER2_READY)
				{
					lobby_state = NOT_READY;
				}
				else if(lobby_state == BOTH_READY)
				{
					lobby_state = PLAYER1_READY;
					fade_condition = false;
				}
			}
			else if(received_signal_type == TYPE_STRING)
			{
				char* temp = (char*) calloc(IM_MESSAGE_MAX + 5,1);
				strcpy(temp, "you: ");
				strcat(temp,received);
				push_back(im_message_log, &temp);
			}
		}
	}
}

int mp_lobby_render()
{
	memcpy(get_program_secondary_buf(),get_background()->color,get_vram_size());
	printgr_shade_centered(get_program_secondary_buf(),"Lobby",get_v_res()/2-200, 0.75,0xFFFF,5,5, .1, 8);
	printgr_shade(get_program_secondary_buf(),get_program_playername(),10, get_v_res()- get_line_space(2), 0.75,0xFFFF,5,5, .1, 2);
	if(*get_date_str_ptr() != NULL)
		printgr_shade_centered(get_program_secondary_buf(),*get_date_str_ptr(),0, 0.75,0xFFFF,5,5, .1, 1);
	if(get_role() == SEARCHING)
	{
		if(get_ticks() % 120 < 30)
			printgr_shade_centered(get_program_secondary_buf(),"Searching",get_v_res()/2, 0.75,0xFFFF,5,5, .1, 7);
		else if(get_ticks() % 120 < 60)
			printgr_shade_centered(get_program_secondary_buf(),"Searching.",get_v_res()/2, 0.75,0xFFFF,5,5, .1, 7);
		else if(get_ticks() % 120 < 90)
			printgr_shade_centered(get_program_secondary_buf(),"Searching..",get_v_res()/2, 0.75,0xFFFF,5,5, .1, 7);
		else
			printgr_shade_centered(get_program_secondary_buf(),"Searching...",get_v_res()/2, 0.75,0xFFFF,5,5, .1, 7);
	}
	else if(get_role() == FAILED)
	{
		printgr_shade_centered(get_program_secondary_buf(),"Connection Failed",get_v_res()/2, 0.75,0xFFFF,5,5, .1, 7);
		printgr_shade_centered(get_program_secondary_buf(),"Player Names must not be the same",get_v_res()/2+100, 0.75,0xFFFF,5,5, .1, 7);
	}
	if(get_role() == CLIENT || get_role() == SERVER)
	{
		strcpy(opponentmessage, "Opponent: ");
		strcat(opponentmessage, get_program_playername2());
		printgr_shade_centered(get_program_secondary_buf(),opponentmessage,get_v_res()/2 +200, 0.75,0xFFFF,5,5, .1, 5);
		draw_cursor(get_program_secondary_buf(),get_cursor());
		size_t i;
		if(lobby_state == PLAYER2_READY)
		{
			lobby_buttons[0]->text = "Play";
			lobby_buttons[0]->trans = 0.9;
		}
		else if(lobby_state == PLAYER1_READY)
		{
			lobby_buttons[0]->text = "Unready";
			lobby_buttons[0]->trans = 0.6;
		}
		else if(lobby_state == BOTH_READY)
		{
			lobby_buttons[0]->text = "Launching";
			lobby_buttons[0]->trans = 0.9;
		}
		else {
			lobby_buttons[0]->text = "Ready";
			lobby_buttons[0]->trans = 0.9;
		}
		lobby_buttons[0]->width = get_string_width(lobby_buttons[0]->text, lobby_buttons[0]->size);
		lobby_buttons[0]->pos.x = get_h_res()/2- lobby_buttons[0]->width/2;
		for(i = 0; i < NUM_LOBBY_BUTTONS; i++)
		{
			draw_button_t_centered(lobby_buttons[i]);
		}
		for(i = 0; i < size(im_message_log) && i < 20; ++i)
		{
			if(**(char**)at(im_message_log,size(im_message_log)-i-1) == 'm')
					printgr_shade(get_program_secondary_buf(),*(char**)at(im_message_log, size(im_message_log)-i-1),20,get_v_res()-100-(i+1)*20, .75,RGB(0,63,16),5,5, .1, 2);
			else printgr_shade(get_program_secondary_buf(),*(char**)at(im_message_log, size(im_message_log)-i-1),20,get_v_res()-100-(i+1)*20, .75,0xFFFF,5,5, .1, 2);
		}
		printgr_shade(get_program_secondary_buf(),">",10,get_v_res()-100, 0.75,0xFFFF,5,5, .1, 2);
		printgr_shade(get_program_secondary_buf(),current_im,20,get_v_res()-100, 0.75,0xFFFF,5,5, .1, 2);
	}
	if(darken > 0)
		darken_buf(get_program_secondary_buf(),darken);
	if(darken > 0 && !fade_condition)
	{

		darken-= FADE_RATE;
	}
	if(fade_condition)
	{
		if(darken <= 127)
		{

			darken += FADE_RATE;
		}
		else
		{
			if(lobby_state == BOTH_READY)
			{

				mp_game();
				fade_condition = false;
				lobby_state = NOT_READY;
			}
			else continue_condition = false;
		}
	}
	copy_video_buffer(get_program_video_mem(), get_program_secondary_buf());
	return 0;
}


int mp_lobby()
{
	int failure = 0;
	if((set_com1_irq_set(generic_ser_subscribe_int_tx_rx(COM1))) < 0)
	{//subscribe interrupts
		printf("mp_lobby(): generic_ser_subscribe_int_tx_rx() failed \n");
		failure = 1;
	}
	if(SET_COMMUNICATION_PARAM())
	{
		printf("ser_set() failed\n");
		return 1;
	}
	initialize_lobby_buttons();
	set_role(SEARCHING);
	lobby_state = NOT_READY;
	enable_per_second_alarm(on_rtc_alarm_int_transmission);
	if(im_message_log != NULL)
		{
			size_t del_i;
			for(del_i = 0 ; del_i < size(im_message_log); del_i++)
			{
				free(*(char**)at(im_message_log, del_i));
			}
			delete_vector_t(im_message_log);
		}
	im_message_log = new_vector_t(sizeof(char*));
	memset(current_im, 0, sizeof(char)*IM_MESSAGE_MAX);
	current_im_index = 0;
	draw_im_index = 0;
	int wait = 0;
	function_to_call = NULL;
	continue_condition = true;
	fade_condition = false;
	darken = FADE_MAX;
	int ipc_status;
	message msg;
	int r;
	set_program_player2name((char*)malloc(sizeof(char)*MAX_PLAYER_NAME_LENGTH+1));
	if(get_program_playername2() == NULL)
		return 1;
	memset(get_program_playername2(),0,sizeof(char)*MAX_PLAYER_NAME_LENGTH+1);
	size_t next_char_index = 0;
	char last_char = 0;
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
					if (msg.NOTIFY_ARG & get_com1_irq_set()) { /* subscribed interrupt */
						if(ser_ih(COM1))
							failure = 1;
					}
					if (msg.NOTIFY_ARG & get_kbd_irq_set()){  /* subscribed kbd interrupt */
						kbd_int_handler();
						if(get_scancode() == ESC_BREAK)
						{
							lobby_state = NOT_READY;
							fade_condition = true;
							transmit_message(MES_BYE);
						}
						if(get_role() == SERVER || get_role() == CLIENT)
						{
							if(get_scancode() == ENTER_MAKE && current_im_index != 0)
							{


								current_im_index = 0;
								transmit_protocol(current_im, TYPE_STRING, strlen(current_im)+1);
								char* temp = (char*) calloc(IM_MESSAGE_MAX + 4,1);
								strcpy(temp, "me: ");
								strcat(temp,current_im);

								push_back(im_message_log, &temp);

								memset(current_im, 0, IM_MESSAGE_MAX);
								memset(transmission_message,0,IM_MESSAGE_MAX+4);

							}
							else
							{
								int action;
								action = process_input(get_scancode(), &last_char);
								if(action == CHAR_RECEIVED)
								{
									if(current_im_index < IM_MESSAGE_MAX -1)
									{
										current_im[current_im_index] = last_char;
										current_im_index++;
									}
								}
								else if(action == BACKSPACE_MAKE)
								{
									if(current_im_index > 0)
									{
										current_im_index--;
										current_im[current_im_index] = '\0';
									}
								}
							}
						}
					}
					if (msg.NOTIFY_ARG & get_rtc_irq_set()) { /* subscribed timer interrupt */
						if(rtc_ih())
							failure = 1;
						getDateString(get_date_str_ptr());
					}
					if(msg.NOTIFY_ARG & get_timer_irq_set()){
						timer_int_handler();
						mp_lobby_render();
						//establish_connection_transmission();
					}
					if (msg.NOTIFY_ARG & get_mouse_irq_set()) { /* subscribed timer interrupt */
						mouse_int_handler();
					}
					break;
				default:
					break; /* no other notifications expected: do nothing */
				}
			} else {
			}
			establish_connection_reception();
			if(mouse_is_updated())
			{
				assign_mouse(get_previous_mouse(), get_mouse());
				assign_mouse(get_mouse(), get_mouse_state());
				if(get_role() == SERVER || get_role() == CLIENT)
					lobby_mouse_event(get_previous_mouse(), get_mouse());
				move_cursor(get_cursor(), get_mouse()->coords);
			}

		} while(continue_condition);
	}
	if(im_message_log != NULL)
	{
		size_t del_i;
		for(del_i = 0 ; del_i < size(im_message_log); del_i++)
		{
			free(*(char**)at(im_message_log, del_i));
		}
		delete_vector_t(im_message_log);
		im_message_log = NULL;
	}
	if(generic_ser_unsubscribe_int_tx_rx(COM1))
	{//unsubscribe interrupts
		printf("ser_int_receive(): generic_ser_unsubscribe_int() failed \n");
		failure = 1;
	}
	disable_per_second_alarm();
	continue_condition = true;
	fade_condition = false;
	darken = FADE_MAX;
	return 0;
}


