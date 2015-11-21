#include "utils.h"



coord_t to_coord_t(int x, int y)
{
	coord_t out;
	out.x= x;
	out.y = y;
	return out;
}

coord_t doubles_to_coord_t(double x, double y)
{
	coord_t out;
	out.x = (int) (x+.5);
	out.y = (int) (y+.5);
	return out;
}
