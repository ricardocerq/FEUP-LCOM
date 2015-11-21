#ifndef GAME_H
#define GAME_H

#include "utils.h"
#include "ppm.h"
#include "ps2mouse.h"

/**
 * @defgroup game game
 * @{
 *
 * implementation of the game
 */

#define ABS_MAX_CELLS 200
#define MAX_CELLS 30
#define MIN_CELLS 29
#define INF_LIM_X 0
#define SUP_LIM_X 1024
#define INF_LIM_Y 0
#define SUP_LIM_Y 768
#define MAX_INITIAL_SPEED 1
#define MIN_INITIAL_SPEED .1
#define MAX_INITIAL_SIZE 100
#define MAX_RADIUS 25
#define MIN_RADIUS 10
#define DISSIPATION_COEF .75
#define CELL_PPM_COUNT 2500
#define SPAWN_DISTANCE_PERCENTAGE 1.25
#define SPAWN_SPEED 1
#define SPAWN_SIZE_PERCENTAGE .05
#define VICTORY_CONDITION .5

typedef enum {PLAYER1, OTHER} cell_type_t;
typedef enum {STARTING, PLAYING, PAUSED, WON, LOST} game_state_t;
typedef enum {SEARCHING, CONNECTING, CONNECTED, SERVER, CLIENT, FAILED} role_t;

typedef struct
{
	cell_type_t type;
	coord_t pos;
	coord_f_t pos_acc;
	coord_f_t v;
	coord_f_t a;
	float radius;
	float area;
	ppm_t* pixmap;
	int alive;
} cell_t;

typedef struct
{
	coord_t pos;
	ppm_t* pixmap;
} cursor_t;

///@brief Subtracts two given coordinates
///@param c1 1st coordinate
///@param c2 2nd coordinate
///@return resulting coordinate
coord_f_t sub_coord_f_t(coord_f_t c1, coord_f_t c2);

///@brief Multiplies a coordinate by a scalar
///@param scalar scalar
///@param c coordinate to multiply
void scalar_mul_coord_f_t(float scalar, coord_f_t* c);

///@brief Adds two given coordinates
///@param c1 1st coordinate
///@param c2 2nd coordinate
///@return resulting coordinate
coord_f_t add_coord_f_t(coord_f_t c1, coord_f_t c2);

///@brief calculates the magnitude of a coordinate
///@param c1 coordinate
///@return magnitude of coordinate
float coord_f_t_magnitude(coord_f_t c1);

///@brief multiplies a coordinate with a scalar
///@param scalar scalar
///@param c1 coordinate to multiply
///@return resulting coordinate
coord_f_t coord_f_t_scalar_mul(float scalar, coord_f_t c1);

///@brief cast coord_t to coord_f_t
///@param c coordinate to cast
///@return cast coordinate
coord_f_t to_coord_f_t(coord_t c);

///@brief set the aceleration for a cell
///@param c cell to modify
void accelerationfunction(cell_t* c);

///@brief Creates a new cell
///@param type type of cell
///@param x position on x
///@param y position on y
///@param vx starting velocity on x
///@param vy starting velocity on y
///@param radius radius of cell
///@return new cell
cell_t* new_cell_t(cell_type_t type, float x, float y, float vx, float vy, float radius);

///@brief advance simulation of one cell
///@param c cell to simulate
///@param timescale difference in time
///@return zero on success, no-zero otherwise
int tick(cell_t* c, float timescale);

///@brief initialize the cursor
///@param c cursor
///@return zero on sucess, 1 otherwise
int initialize_cursor(cursor_t* c);

///@brief move the cursor
///@param c cursor
///@param move coordinate of mouse delta
void move_cursor(cursor_t* c, coord_t move);

///@brief calculate index of cell with the specified radius in the pixamp array
///@param radius radius of cell
///@return index of pixmap in array
int ppm_cell_index(int radius);

///@brief initialize all the cells
///@param cells array of cells
///@return number of cells initialized
int initializecells(cell_t* cells[]);

///@brief advance the simulation of all the cells
///@param cells array of cells
///@param timescale difference in time
///@param numcells number of cells
///@return zero on success, no-zero otherwise
int tick_cells(cell_t* cells[], float timescale, int numcells);

///Calculates area of intersection between two circles
///@param d distance between centers
///@param R radius of biggest circle
///@param r radius of smallest circle
float area_intersection(float d, float R, float r);

///@brief calculates distance between two points
///@param pos1 1st coordinate
///@param pos2 2nd coordinate
///@return distance between coordinates
float distance(coord_t pos1, coord_t pos2);

///@brief Resolve the collision between two cells
///@param c1 1st cell
///@param c2 2nd cell
///@return zero on success, non-zero if one of the cells doesn't exist
int collision_two_cells(cell_t* c1, cell_t* c2);

///@brief resolve the collision of cells
///@param cells array of cells
///@param numcells number of cells
///@return zero on sucess, no-zero if of the cells doesn't exist
int collision_cells(cell_t* cells[], int* numcells);

///@brief draw all cells
///@param buf buffer to draw on
///@param cells array of cells
///@param numcells number of cells
///@return zero on success, no-zero otherwise
int draw_cells(char* buf, cell_t* cells[], int numcells);

///@brief resolves the spawning of a cell by the player
///@param player1 cell of player
///@param cursor cursor
///@return zero on success, no-zero otherwise
int player_spawn_cell(cell_t* player1, cursor_t* cursor);

///@brief resolve a mouse event
///@param previous_mouse previous state of mouse
///@param mouse current state of mouse
void game_mouse_event(mouse_state_t* previous_mouse,mouse_state_t* mouse);

///@brief Renders the single player game
///@return zero on success, no-zero otherwise
int sp_game_render();

///@brief main single player game function
///@return zero on success, no-zero otherwise
int sp_game();

///@brief Receive information in multiplayer game
///@return zero on success, no-zero otherwise
int mp_receive();

///@brief Render multiplayer game
///@return zero on success, no-zero otherwise
int mp_game_render();

///@brief Main multiplayer game function
///@return zero on success, no-zero otherwise
int mp_game();


/** @} end of game */

#endif
