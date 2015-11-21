#ifndef UTILS_H
#define UTILS_H

/**
 * @defgroup utils utils
 * @{
 *
 * general utilities
 */

#define PI 3.14159265358979323846264338327950288419716939937510582

///@brief floating point coordinate structure
typedef struct
{
	float x;
	float y;
} coord_f_t; //floating point coordinates

///@brief coordinate struct
typedef struct
{
	int x;
	int y;
} coord_t;


///@brief convert two coordinate
///@param x x coordinate
///@param y y coordinate
///@return resulting coordinate
coord_t to_coord_t(int x, int y);


///@brief convert two coordinates
///@param x x coordinate
///@param y y coordinate
///@return resulting coordinates
coord_t doubles_to_coord_t(double x, double y);

/** @} end of utils */

#endif
