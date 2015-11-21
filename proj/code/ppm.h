#ifndef PPM_H
#define PPM_H

#include <stdlib.h>

/**
 * @defgroup ppm ppm
 * @{
 *
 * read and manipulate images in ppm format
 */

///@brief image struct
typedef struct
{
	int height, width;
	unsigned short* color;
	unsigned short* alpha;
} ppm_t;

///delete a ppm
///@param p ppm to delete
void delete_ppm_t(ppm_t * p);

///@brief write a pixel to a ppm
///@param buf buffer
///@param x position
///@param y poition
///@param color color to write
///@param height height of pixmap
///@param width width of pixmap
///@return zero on sucess, non-zero otherwise
int write_pixelxy_ppm(unsigned short* buf,int x, int  y, unsigned short color, int height, int width);

///@brief read a pixel from a ppm
///@param buf buffer
///@param x position
///@param y poition
///@param out return value
///@param height height of pixmap
///@param width width of pixmap
///@return zero on sucess, non-zero otherwise
int read_pixelxy_ppm(unsigned short * buf, int x, int y, unsigned short * out, int height, int width);

///@brief write a pixel to a ppm, without checking boundaries
///@param buf buffer
///@param x position
///@param y poition
///@param color color to write
///@param height height of pixmap
///@param width width of pixmap
///@return zero on sucess, non-zero otherwise
void unchecked_write_pixelxy_ppm(unsigned short* buf,int x, int  y, unsigned short color, int height, int width);

///@brief read a pixel from a ppm, without checking boundaries
///@param buf buffer
///@param x position
///@param y poition
///@param out return value
///@param height height of pixmap
///@param width width of pixmap
///@return zero on sucess, non-zero otherwise
void unchecked_read_pixelxy_ppm(unsigned short * buf, int x, int y, unsigned short * out, int height, int width);

///@brief get the average alpha of a block, for use in reduction
///@param alpha pixmap
///@param j start of block
///@param i start of block
///@param height height of block
///@param width width of block
///@param factor factor of reduction
///@return average alpha
unsigned short get_block_alpha(unsigned short* alpha,int j, int i,int height,int width,int factor);

///@brief get the average color of a block, for use in reduction
///@param color pixmap
///@param j start of block
///@param i start of block
///@param height height of block
///@param width width of block
///@param factor factor of reduction
///@return average color
unsigned short get_block_color(unsigned short* color,int j, int i,int height,int width,int factor);

///@brief write a block of a color, for use in scaling
///@param buf buffer to fill
///@param x start of block
///@param y start of block
///@param size size of block
///@param color color of block
///@param height height of block
///@param width width of block
///@return average color
int write_block(short* buf, int x,int y, int size, unsigned short color, int height, int width);

///increase a size of a ppm
///@param original original ppm
///@param factor factor of increase
///@return scaled ppm
ppm_t* increase_ppm(ppm_t * original, int factor);

///@brief get the average alpha of a block, for use in reduction
///@param alpha pixmap
///@param j start of block
///@param i start of block
///@param height height of block
///@param width width of block
///@param factor factor of reduction
///@return average alpha
unsigned short get_block_alpha_float(unsigned short* alpha,double j, double i,int height,int width,int factor);

///@brief get the average color of a block, for use in reduction
///@param color pixmap
///@param j start of block
///@param i start of block
///@param height height of block
///@param width width of block
///@param factor factor of reduction
///@return average color
unsigned short get_block_color_float(unsigned short* color,double j, double i,int height,int width,int factor);

///@brief allocate space for a reduced ppm
///@param original original ppm
///@param factor factor of reduction
///@return reduced ppm
ppm_t* reduce_ppm_no_fill(ppm_t* original, double factor);

///@brief fill a reduced ppm for which space has been allocated
///@param original original ppm
///@param out filled reduced ppm
void fill_reduced_ppm(ppm_t * original, ppm_t* out);


///@brief read ppm from a file
///@param colorfile path containing picture's color information
///@param alphafile path containing picture's transparency information
///@return resulting picture
ppm_t* read_ppm(char* colorfile, char* alphafile);

///@brief reduce ppm_t by a floating point factor
///@param original original ppm
///@param factor factor of reduction
///@return reduced ppm
ppm_t* reduce_float(ppm_t * original, double factor);

///@brief get the average color of a block, for use in reduction, optimized by reading a number of pixel that is a power of 2, division is calculated using shifts
///@param color target pixmap
///@param j start of block
///@param i start of block
///@param height height of block
///@param width width of block
///@param factor factor of reduction
///@param pix_pow_2 target power of 2
///@return average color
unsigned short get_block_color_o(unsigned short* color,int j, int i,int height,int width,int factor,int pix_pow_2);

///@brief get the average alpha of a block, for use in reduction, optimized by reading a number of pixel that is a power of 2, division is calculated using shifts
///@param alpha target pixmap
///@param j start of block
///@param i start of block
///@param height height of block
///@param width width of block
///@param factor factor of reduction
///@param pix_pow_2 target power of 2
///@return average color
unsigned short get_block_alpha_o(unsigned short* alpha,int j, int i,int height,int width,int factor,int pix_pow_2);

///@brief fill reduced ppm, for which space has been allocated, optimized
///@param original original original pixmap
///@param out filled reduced ppm
///@return filled reduced ppm
ppm_t* fill_reduced_ppm_o(ppm_t * original,ppm_t* out);

///@brief rotate ppm
///@param original original pixmap
///@param angle angle of rotation
///@return rotated ppm
ppm_t* rotate_ppm(ppm_t* original, int angle);

///@brief turns pixmap output by photoshop (6 bytes per pixel, 3 for color (24 bit), 3 for checksum) into pixmap usable by program(16 bit color)
///assumes picture is in grayscale maps black to 0 white to MAX_ALPHA (127)
///@param uncompressed pixmap read from file
///@param target pixmap to fill
///@param numpixels number of pixels
///@return Return 0 upon success and non-zero otherwise
int compressalpha(unsigned char* uncompressed, unsigned short* target, size_t numpixels);

///@brief turns pixmap output by photoshop (6 bytes per pixel, 3 for color (24 bit), 3 for checksum) into pixmap usable by program(16 bit color)
///@param uncompressed pixmap read from file
///@param target pixmap to fill
///@param numpixels number of pixels
///@return Return 0 upon success and non-zero otherwise
int compresscolor(unsigned char* uncompressed, unsigned short* target,size_t numpixels);

///@brief function to read a pixmap from a file
///@param file to read
///@param w width of pixmap read
///@param h height of pixmap read
///@param iscolor indicates whether file contains color information or transparency information
///@return read pixmap
unsigned short* read_ppm_map(char* file, int* w, int* h, char iscolor);

///@brief read ppm from a file, with no transparency information
///@param colorfile path containing pixmap color information
///@return resulting pixmap
ppm_t* read_ppm_noAlpha(char* colorfile);

/** @} end of ppm */

#endif
