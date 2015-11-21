#include "game.h"
#include "video_gr.h"
#include "mouse.h"
#include "ppm.h"
#include "vector.h"
#include "i8042.h"
#include "i8254.h"
#include "font.h"
#include "graphics_mode.h"
#include "utils.h"
#include "rtc.h"
#include "program.h"
#include "menu.h"
#include "ser_port.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>




static cell_t* player1;
static cell_t* cells[ABS_MAX_CELLS+1];
static game_state_t game_state = STARTING;
static int numcells = 0;
static float gamespeed = 1;
static unsigned long gametime = 0;
static float totalarea = 0;
static bool continue_condition = true;
static bool fade_condition = false;
static int darken = 0;

static float three_trans = .6;
static float two_trans = .6;
static float one_trans = .6;
static float go_trans = .6;
static char playertime[50];

static game_state_t player2_state = PLAYING;
static unsigned long player2_gametime = 0;
static char player2timemessage[50];


#define maxcell(c1, c2) ((c2->radius > c1->radius)? c2: c1)
#define mincell(c1, c2) ((c2->radius <= c1->radius)? c2: c1)
#define absolute(n1) ((n1<0)? -n1:n1)


coord_f_t sub_coord_f_t(coord_f_t c1, coord_f_t c2)
{
	coord_f_t out;
	out.x = c1.x - c2.x;
	out.y = c1.y - c2.y;
	return out;
}
void scalar_mul_coord_f_t(float scalar, coord_f_t* c)
{
	c->x *= scalar;
	c->y *= scalar;
}
coord_f_t add_coord_f_t(coord_f_t c1, coord_f_t c2)
{
	coord_f_t out;
	out.x = c1.x + c2.x;
	out.y = c1.y + c2.y;
	return out;
}
float coord_f_t_magnitude(coord_f_t c1)
{
	return sqrt(c1.x*c1.x+c1.y*c1.y);
}
coord_f_t coord_f_t_scalar_mul(float scalar, coord_f_t c1)
{
	coord_f_t out;
	out.x = scalar * c1.x;
	out.y = scalar * c1.y;
	return out;
}

coord_f_t to_coord_f_t(coord_t c)
{
	coord_f_t out;
	out.x = c.x;
	out.y= c.y;
	return out;
}
void accelerationfunction(cell_t* c)
{
	c->a.x = 0;
	c->a.y = 0;
}















cell_t* new_cell_t(cell_type_t type, float x, float y, float vx, float vy, float radius)
{
	cell_t * out = (cell_t*) malloc(sizeof(cell_t));
	if(out == NULL)
		return NULL;
	out->type = type;
	out->pos_acc.x = x;
	out->pos_acc.y = y;
	out->pos.x = x;
	out->pos.y = y;
	out->v.x = vx;
	out->v.y = vy;
	out->radius = radius;
	out->area = PI * pow((float)radius,2);
	totalarea += out->area;
	out->pixmap = get_cell_pixmap(out->radius);
	out->alive = 1;
	accelerationfunction(out);
	return out;
}

int tick(cell_t* c, float timescale)
{
	c->pos_acc.x += c->v.x*timescale;
	c->pos_acc.y += c->v.y*timescale;
	c->pos.x = c->pos_acc.x;
	c->pos.y = c->pos_acc.y;
	if((c->pos.x+c->radius > SUP_LIM_X && c->v.x > 0) || (c->pos.x - c->radius < INF_LIM_X && c->v.x < 0))
		{
			c->v.x = - c->v.x;
		}
	if((c->pos.y+c->radius > SUP_LIM_Y && c->v.y > 0) || (c->pos.y - c->radius < INF_LIM_Y&& c->v.y < 0))
		{
			c->v.y = - c->v.y;
		}
	c->v.x 	 +=  c->a.x*timescale;
	c->v.y 	 +=  c->a.y*timescale;
	accelerationfunction(c);
	return 0;
}

int initialize_cursor(cursor_t* c)
{
	ppm_t* or = read_ppm(get_dir("/ppms/cursor2.ppm"),get_dir("/ppms/cursor2.ppm"));
	if(or == NULL)
		return 1;
	c->pixmap = or;
	size_t i;
	c->pos.x  =SUP_LIM_X/2;
	c->pos.y = SUP_LIM_Y/2;
	return 0;
}
void move_cursor(cursor_t* c, coord_t move)
{
	static int start = 1;
	if(start > 0)
		{start--;return;}
	if(c->pos.x + move.x >= 0 && c->pos.x + move.x < get_h_res())
		c->pos.x += move.x;
	if(c->pos.y - move.y >= 0 && c->pos.y - move.y < get_v_res())
		c->pos.y -= move.y;
	int x = c->pos.x - get_h_res()/2;
	int y = c->pos.y - get_v_res()/2;
}


















int ppm_cell_index(int radius)
{
	return (radius-1 < 249)? radius-1:249;
}
int initializecells(cell_t* cells[])
{
	int numcells = rand() % (MAX_CELLS-MIN_CELLS)+MIN_CELLS;
	size_t i = 0;
	cell_type_t type = PLAYER1;
	for(;i < numcells; i++)
	{
		int radius;
		if(type == PLAYER1)
			radius = 25;//(MAX_RADIUS+MIN_RADIUS+MIN_RADIUS)*1.25;
		else radius = rand() % (MAX_RADIUS-MIN_RADIUS)+ MIN_RADIUS;
		int x = rand() % (SUP_LIM_X - radius - (INF_LIM_X +radius)) +(INF_LIM_X +radius);
		int y = rand() % (SUP_LIM_Y - radius - (INF_LIM_Y +radius)) +(INF_LIM_Y +radius);
		//float vx = (float)((rand() % (int)(MAX_INITIAL_SPEED*1000)))/1000+.1;
		//float vy = (float)((rand() % (int)(MAX_INITIAL_SPEED*1000)))/1000+.1;
		float vx = ((float)(rand() % (int)((MAX_INITIAL_SPEED-MIN_INITIAL_SPEED)*1000)))/1000+ MIN_INITIAL_SPEED;
		if(rand() % 2 == 0)
			vx *= -1;
		float vy = ((float)(rand() % (int)((MAX_INITIAL_SPEED-MIN_INITIAL_SPEED)*1000)))/1000+ MIN_INITIAL_SPEED;
		if(rand() % 2 == 0)
			vy *= -1;
		cell_t * c = new_cell_t(type, x, y, vx, vy,radius);
		accelerationfunction(c);
		if(type == PLAYER1)
			player1 = c;
		//c->pixmap = get_cell_pixmap(c->radius);


		cells[i] = c;
		c->type = type;
		type = OTHER;
	}
	return numcells;
}

int tick_cells(cell_t* cells[], float timescale, int numcells)
{
	size_t i =0;
	for(; i < numcells; i++)
	{
		tick(cells[i], timescale);
	}
	return 0;
}

float area_intersection(float d, float R, float r)
{
	float out =  sqrt((-d+r+R)*(d+r-R)*(d-r+R)*(d+r+R))/2;
	return out;
}

float distance(coord_t pos1, coord_t pos2)
{
	return (float) sqrt((float)((pos1.x-pos2.x)*(pos1.x-pos2.x)+(pos1.y-pos2.y)*(pos1.y-pos2.y)));
}


int collision_two_cells(cell_t* c1, cell_t* c2)
{
	if(c1 == NULL || c2 == NULL)
		return 1;
	if(c1->radius < 1 || c2->radius < 1 || c1 == c2)
		return 0;
	if(absolute(c1->pos.x - c2->pos.x) > c1->radius + c2->radius)
		return 0;
	if(absolute(c1->pos.y - c2->pos.y) > c1->radius + c2->radius)
		return 0;
	float dist = distance(c1->pos, c2->pos);
	if(dist < (c1->radius+c2->radius))
	{
		cell_t* max = maxcell(c1,c2);
		cell_t* min = mincell(c1,c2);
		float area_difference;
		if(dist < max->radius)
			area_difference= min->area;
		else area_difference= area_intersection(dist, max->radius, min->radius);
		min->area -= area_difference;
		min->radius = sqrt(min->area /PI);
		coord_f_t vr = coord_f_t_scalar_mul(1/(min->area+max->area),sub_coord_f_t(coord_f_t_scalar_mul(max->area,max->v), coord_f_t_scalar_mul(min->area,min->v)));
		min->v = sub_coord_f_t(min->v,coord_f_t_scalar_mul(.05,vr));
		max->v = sub_coord_f_t(max->v,coord_f_t_scalar_mul(.05,vr));
		if(min->radius > 0)
		min->pixmap = get_cell_pixmap(min->radius);
		max->area += area_difference;
		max->radius = sqrt(max->area /PI);
		max->pixmap = get_cell_pixmap(max->radius);
	}
	return 0;
}

int collision_cells(cell_t* cells[], int* numcells)
{
	size_t i;
	size_t j;
	for(i=0; i < *numcells - 1; i++)
	{
		for(j=i; j < *numcells; j++)
		{
			collision_two_cells(cells[i], cells[j]);
		}
	}
	for(i=0; i < *numcells; i++)
	{
		if((cells[i]->radius) < 1)//remove element
		{
			if(cells[i]->type != PLAYER1)
			{
				free(cells[i]);cells[i] = NULL;}
			memcpy((void*)(cells+i), (void*) (cells+i+1), (*numcells - i - 1)* sizeof(cell_t*));
			i--;
			(*numcells)--;
		}
	}
	return 0;
}
int draw_cells(char* buf, cell_t* cells[], int numcells)
{
	size_t i =0;
	for(; i < numcells; i++)
	{
		cell_t * current = cells[i];
		unsigned short color;
		if(current->type == PLAYER1)
			draw_ppm_alpha_centered_player(buf,current->pos.x,current->pos.y,current->pixmap);
		else
			{
				draw_ppm_alpha_centered_enemy(buf,current->pos.x,current->pos.y,current->pixmap, (player1->radius <= 0)? 16 : (int)(current->radius / player1->radius));
			}
	}
	return 0;
}



int player_spawn_cell(cell_t* player1, cursor_t* cursor)
{
	if(numcells >= ABS_MAX_CELLS)
		return 1;
	if(player1->radius < 1)
		return 1;
	coord_f_t relative_pos_u = sub_coord_f_t(to_coord_f_t(get_cursor()->pos), player1->pos_acc);
	relative_pos_u = coord_f_t_scalar_mul(1/coord_f_t_magnitude(relative_pos_u), relative_pos_u);
	coord_f_t pos_cell = add_coord_f_t(player1->pos_acc, coord_f_t_scalar_mul(SPAWN_DISTANCE_PERCENTAGE * player1->radius, relative_pos_u));
	if(pos_cell.x < INF_LIM_X || pos_cell.x >= SUP_LIM_X || pos_cell.y < INF_LIM_Y || pos_cell.y >= SUP_LIM_Y)
		return 1;
	cell_t* newcell = (cell_t*) malloc(sizeof(cell_t));
	if(newcell == NULL)
		{printf("cell malloc failed\n");return 0;}
	newcell->pos_acc = pos_cell;
	newcell->v = coord_f_t_scalar_mul(SPAWN_SPEED, relative_pos_u);
	newcell->area = SPAWN_SIZE_PERCENTAGE*player1->area;
	newcell->radius = sqrt(newcell->area /PI);
	newcell->pixmap = get_cell_pixmap(newcell->radius);
	newcell->type = OTHER;
	newcell->a.x = newcell->v.x;
	newcell->a.y = newcell->v.y;
	newcell->pos.x = (int) newcell->pos_acc.x;
	newcell->pos.y = (int) newcell->pos_acc.y;
	player1->area = player1->area - SPAWN_SIZE_PERCENTAGE*player1->area;
	player1->radius = sqrt(player1->area /PI);
	player1->pixmap = get_cell_pixmap(player1->radius);
	player1->v = sub_coord_f_t(player1->v, newcell->v);
	cells[numcells] = newcell;
	numcells++;
	return 0;
}

void game_mouse_event(mouse_state_t* previous_mouse,mouse_state_t* mouse)
{
	if(game_state != PAUSED && game_state != STARTING)
	if(get_mouse()->lb == 1 && get_previous_mouse()->lb == 0)
	{
		player_spawn_cell(player1, get_cursor());
	}
}

int sp_game_render()
{
	memcpy(get_program_secondary_buf(),get_background()->color,get_vram_size());

	draw_cursor(get_program_secondary_buf(),get_cursor());

	draw_cells(get_program_secondary_buf(),cells, numcells);

	if(*get_date_str_ptr() != NULL)
		printgr_shade_centered(get_program_secondary_buf(),*get_date_str_ptr(),0, 0.75,0xFFFF,5,5, .1, 1);
	if(game_state != WON && game_state != LOST)
	{
		sprintf(playertime, "%.2f s", (float)gametime/60);
		printgr_shade_centered(get_program_secondary_buf(),playertime,get_v_res()-get_line_space(1), .75 ,0xFFFF,5,5, .1, 1);
	}
	if(game_state == PAUSED)
		printgr_shade_centered(get_program_secondary_buf(),"Paused",get_v_res()/2-100, .75 ,0xFFFF,5,5, .1, 10);

	if(game_state== WON)
	{
		printgr_shade_centered(get_program_secondary_buf(),":D",get_v_res()/2-100, .75 ,0xFFFF,5,5, .1, 10);
		printgr_shade_centered(get_program_secondary_buf(),playertime,get_v_res()/2+100, .75 ,0xFFFF,5,5, .1, 5);
	}
	else if(game_state== LOST)
	{
		printgr_shade_centered(get_program_secondary_buf(),":(",get_v_res()/2-100, .75 ,0xFFFF,5,5, .1, 10);
		printgr_shade_centered(get_program_secondary_buf(),playertime,get_v_res()/2+100, .75 ,0xFFFF,5,5, .1, 5);
	}
	else if(game_state == STARTING)
	{
		if(three_trans > 0)
		{
			printgr_shade_centered(get_program_secondary_buf(),"3",get_v_res()/2-100, three_trans ,0xFFFF,5,5, .1, 10);
			three_trans -= 0.01;
		}
		else if(two_trans > 0)
		{
			printgr_shade_centered(get_program_secondary_buf(),"2",get_v_res()/2-100,two_trans,0xFFFF,5,5, .1, 10);
			two_trans -= 0.01;
		}
		else if(one_trans > 0)
		{
			printgr_shade_centered(get_program_secondary_buf(),"1",get_v_res()/2 -100,one_trans,0xFFFF,5,5, .1, 10);
			one_trans -= 0.01;
		}
		else game_state = PLAYING;
	}
	else if(go_trans > 0)
	{
		printgr_shade_centered(get_program_secondary_buf(),"GO",get_v_res()/2 -100,go_trans,0xFFFF,5,5, .1, 10);
		go_trans -= 0.01;
	}
	if(game_state == PLAYING)
		gametime++;


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
}

int sp_game()
{
		continue_condition = true;
		fade_condition = false;
		darken = 127;
		numcells = 0;
		gamespeed = 1;
		totalarea = 0;
		gametime = 0;
		numcells = initializecells(cells);
		int failure = 0;
		int ipc_status;
		message msg;
		int r;
		//char* get_program_secondary_buf() = (char*) get_get_program_secondary_buf()();
		three_trans = .6;
		two_trans = .6;
		one_trans = .6;
		go_trans = .6;
		game_state = STARTING;
		unsigned long exit_condition = 0;
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
							if(!(get_scancode()&BREAK_CODE_BIT) && get_scancode() != ESC_MAKE)
							{
							if(game_state == PLAYING)
								game_state = PAUSED;
							else if(game_state == PAUSED)
								game_state = PLAYING;
							}
							if(get_scancode() == ESC_BREAK)
								fade_condition = true;
							//exit_condition = get_scancode();
						}
						if (msg.NOTIFY_ARG & get_rtc_irq_set()) { /* subscribed timer interrupt */

							if(rtc_ih())
								failure = 1;
							getDateString(get_date_str_ptr());
						}
						if(msg.NOTIFY_ARG & get_timer_irq_set()){
							timer_int_handler();
							sp_game_render();
							if(game_state != STARTING && game_state != PAUSED)
								tick_cells(cells,1, numcells);
							collision_cells(cells,&numcells);
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
				if(game_state == PLAYING)
				{
					if(player1->area > totalarea * VICTORY_CONDITION)
						game_state = WON;
					if(player1-> area <= 0)
						game_state = LOST;
				}
				if(mouse_is_updated())
				{
					assign_mouse(get_previous_mouse(), get_mouse());
					assign_mouse(get_mouse() ,get_mouse_state());
					game_mouse_event(get_previous_mouse(), get_mouse());
					move_cursor(get_cursor(), get_mouse()->coords);
				}
			} while(continue_condition);
		}
		return failure;
}


int mp_receive()
{
	unsigned long received = 0;
	signal_type_t received_signal_type;
	size_t received_size = 0;
	if(receive_protocol(&received,&received_signal_type, &received_size) == 0)
	{
		if(received_signal_type == TYPE_MESSAGE)
		{
			if((unsigned char)received == MES_BYE)
				fade_condition = true;
			else if((unsigned char)received == MES_WON)
			{
				player2_state = WON;
				while(receive_protocol(&received,&received_signal_type, &received_size) != 0)
				{}
				player2_gametime = received;
				sprintf(player2timemessage, "%s\'s time: %.2f s", get_program_playername2(), (float)player2_gametime/60);
			}
			else if((unsigned char)received == MES_LOST)
			{
				player2_state = LOST;
				while(receive_protocol(&received,&received_signal_type, &received_size) != 0)
				{}
				player2_gametime = received;
				sprintf(player2timemessage, "%s\'s time: %.2f s", get_program_playername2(), (float)player2_gametime/60);
			}
		}
	}
}
int mp_game_render()
{
	memcpy(get_program_secondary_buf(),get_background()->color,get_vram_size());
	draw_cursor(get_program_secondary_buf(),get_cursor());
	draw_cells(get_program_secondary_buf(),cells, numcells);
	if(*get_date_str_ptr() != NULL)
		printgr_shade_centered(get_program_secondary_buf(),*get_date_str_ptr(),0, 0.75,0xFFFF,5,5, .1, 1);
	if(game_state != WON && game_state != LOST)
	{
		sprintf(playertime, "%.2f s", (float)gametime/60);
		printgr_shade_centered(get_program_secondary_buf(),playertime,get_v_res()-get_line_space(1), .75 ,0xFFFF,5,5, .1, 1);
	}
	if((game_state == LOST && player2_state != LOST) || (game_state == LOST && player2_state == LOST && gametime <  player2_gametime) || (game_state != WON && player2_state == WON)|| (game_state == WON && player2_state== WON && gametime < player2_gametime))
	{
		printgr_shade_centered(get_program_secondary_buf(),":(",get_v_res()/2-100, .75 ,0xFFFF,5,5, .1, 10);
		if(game_state != PLAYING)
		printgr_shade_centered(get_program_secondary_buf(),playertime,get_v_res()/2+100, .75 ,0xFFFF,5,5, .1, 5);
		if(player2_state == LOST || player2_state == WON)
		printgr_shade_centered(get_program_secondary_buf(),player2timemessage,get_v_res()/2+150, .75 ,0xFFFF,5,5, .1, 5);
	}
	else if((game_state == WON && player2_state != WON) || (game_state == WON && player2_state== WON && gametime < player2_gametime)|| (game_state != LOST && player2_state == LOST)|| (game_state == LOST && player2_state == LOST && gametime < player2_gametime))
	{
		printgr_shade_centered(get_program_secondary_buf(),":D",get_v_res()/2-100, .75 ,0xFFFF,5,5, .1, 10);
		if(game_state != PLAYING)
		printgr_shade_centered(get_program_secondary_buf(),playertime,get_v_res()/2+100, .75 ,0xFFFF,5,5, .1, 5);
		if(player2_state == LOST || player2_state == WON)
			printgr_shade_centered(get_program_secondary_buf(),player2timemessage,get_v_res()/2+150, .75 ,0xFFFF,5,5, .1, 5);
	}
	else if(game_state == STARTING)
	{
		if(three_trans > 0)
		{
			printgr_shade_centered(get_program_secondary_buf(),"3",get_v_res()/2-100, three_trans ,0xFFFF,5,5, .1, 10);
			three_trans -= 0.01;
		}
		else if(two_trans > 0)
		{
			printgr_shade_centered(get_program_secondary_buf(),"2",get_v_res()/2-100,two_trans,0xFFFF,5,5, .1, 10);
			two_trans -= 0.01;
		}
		else if(one_trans > 0)
		{
			printgr_shade_centered(get_program_secondary_buf(),"1",get_v_res()/2 -100,one_trans,0xFFFF,5,5, .1, 10);
			one_trans -= 0.01;
		}
		else game_state = PLAYING;
	}
	else if(go_trans > 0)
	{
		printgr_shade_centered(get_program_secondary_buf(),"GO",get_v_res()/2 -100,go_trans,0xFFFF,5,5, .1, 10);
		go_trans -= 0.01;
	}
	if(darken > 0)
		darken_buf(get_program_secondary_buf(),darken);
	if(darken > 0 && !fade_condition)
		darken-= FADE_RATE;
	if(fade_condition)
	{
		if(darken <= 127)
			darken += FADE_RATE;
		else continue_condition = false;
	}
	copy_video_buffer(get_program_video_mem(), get_program_secondary_buf());
	return 0;
}
int mp_game()
{
	continue_condition = true;
	fade_condition = false;
	darken = 127;
	numcells = 0;
	gamespeed = 1;
	totalarea = 0;
	gametime = 0;
	player2_state = PLAYING;
	player2_gametime = 0;
	numcells = initializecells(cells);
	int failure = 0;
	int ipc_status;
	message msg;
	int r;
	//char* get_program_secondary_buf() = (char*) get_get_program_secondary_buf()();
	three_trans = .6;
	two_trans = .6;
	one_trans = .6;
	go_trans = .6;
	game_state = STARTING;
	unsigned long exit_condition = 0;
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
						if(get_scancode() == ESC_BREAK)
							{
								fade_condition = true;
								transmit_message(MES_BYE);
							}
						//exit_condition = get_scancode();
					}
					if (msg.NOTIFY_ARG & get_com1_irq_set()) { /* subscribed interrupt */
						if(ser_ih(COM1))
							failure = 1;
					}
					if (msg.NOTIFY_ARG & get_rtc_irq_set()) { /* subscribed timer interrupt */
						if(rtc_ih())
							failure = 1;
						getDateString(get_date_str_ptr());
					}
					if(msg.NOTIFY_ARG & get_timer_irq_set()){
						timer_int_handler();
						if(game_state == PLAYING)
							gametime++;
						if(game_state != STARTING && game_state != PAUSED)
							tick_cells(cells,1, numcells);

						collision_cells(cells,&numcells);
						mp_game_render();
						mp_receive();
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
			if(game_state == PLAYING)
			{
				if(player1->area > totalarea * VICTORY_CONDITION)
				{
					transmit_message(MES_WON);
					transmit_protocol(&gametime, TYPE_INT, sizeof(unsigned long));
					game_state = WON;
				}
				if(player1-> area <= 0)
				{
					transmit_message(MES_LOST);
					transmit_protocol(&gametime, TYPE_INT, sizeof(unsigned long));
					game_state = LOST;
				}
			}
			if(mouse_is_updated())
			{
				assign_mouse(get_previous_mouse(), get_mouse());
				assign_mouse(get_mouse(),get_mouse_state());
				game_mouse_event(get_previous_mouse(), get_mouse());
				move_cursor(get_cursor(), get_mouse()->coords);
			}
		} while((continue_condition));
	}
	return failure;
}



