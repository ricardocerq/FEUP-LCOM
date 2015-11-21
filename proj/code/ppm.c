#include "ppm.h"
#include "graphics_mode.h"
#include "i8042.h"
#include <stdio.h>
#include <math.h>
#include "ps2mouse.h"
#include "utils.h"

void delete_ppm_t(ppm_t * p)
{
	if(p!=NULL)
	{
		if(p->color != NULL)
			free(p->color);
		if(p->alpha != NULL)
			free(p->alpha);
		free(p);
	}
}
int write_pixelxy_ppm(unsigned short* buf,int x, int  y, unsigned short color, int height, int width)
{
	if(x < 0 || x >= width || y < 0 || y >=height)
		return 1;
	unsigned short* pixel = (buf + y *  width + x);
	*pixel = color;
	return 0;
}
int read_pixelxy_ppm(unsigned short * buf, int x, int y, unsigned short * out, int height, int width)
{
	if(x < 0 || x >=width || y < 0 || y >=height)
		return 1;
	*out = *(buf + y * width+ x);
	return 0;
}

void unchecked_write_pixelxy_ppm(unsigned short* buf,int x, int  y, unsigned short color, int height, int width)
{
	*(unsigned short *)(buf + y *  width + x) = color;
}
void unchecked_read_pixelxy_ppm(unsigned short * buf, int x, int y, unsigned short * out, int height, int width)
{
	*out = *(buf + y * width+ x);
}

unsigned short get_block_alpha(unsigned short* alpha,int j, int i,int height,int width,int factor)
{
	unsigned short out;
	unsigned long totalalpha=0;
	unsigned short pixelvalue=0;
	size_t count=0;
	int x;
	int y;
	for( y = i; y < i + factor; y++)
	{
		for(x = j; x < j + factor; x++)
		{
			if(read_pixelxy_ppm(alpha, x, y, &pixelvalue, height, width) == 0)
			{
				count++;
				totalalpha += pixelvalue;
			}
		}
	}
	totalalpha /= count;
	out = totalalpha;
	return out;
}
unsigned short get_block_color(unsigned short* color,int j, int i,int height,int width,int factor)
{
	unsigned short out;
	unsigned long totalred=0;
	unsigned long totalgreen=0;
	unsigned long totalblue=0;
	unsigned short pixelvalue=0;
	size_t count=0;
	int x;
	int y;
	for( y = i; y < i + factor; y++)
	{
		for(x = j; x < j + factor; x++)
		{
			if(read_pixelxy_ppm(color, x, y, &pixelvalue, height, width) == 0)
			{
				count++;
				totalred += GET_R(pixelvalue);
				totalgreen += GET_G(pixelvalue);
				totalblue += GET_B(pixelvalue);
			}
		}
	}
	totalred /= count;
	totalgreen /= count;
	totalblue /= count;
	out = RGB(totalred,	totalgreen,totalblue);
	return out;
}

int write_block(short* buf, int x,int y, int size, unsigned short color, int height, int width)
{
	coord_t pos;
	pos.x = x;
	pos.y = y;
	unsigned short i = 0;
	unsigned short j = 0;
	for(; i < size; i++)
	{
		for(; j < size; j++)
		{
			write_pixelxy_ppm(buf,pos.x, pos.y,color, height, width);
			pos.x++;
		}
		j = 0;
		pos.y++;
		pos.x = x;
	}
	return 0;
}
ppm_t* increase_ppm(ppm_t * original, int factor)
{
	if(factor <= 1)
		return original;
	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
	out->height = original->height*factor;
	out->width = original->width*factor;
	out->color = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	out->alpha = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	size_t i;
	size_t j;
	for(i=0; i < out->height; i++)
	{
		for(j = 0; j < out->width; j++)
		{
			unsigned short currentcolor;
			read_pixelxy_ppm(original->color, j, i, &currentcolor, original->height, original->width);
			write_block(out->color, j*factor, i*factor, factor, currentcolor, out->height, out->width);
		}
	}
	for(i=0; i < out->height; i++)
	{
		for(j = 0; j < out->width; j++)
		{
			unsigned short currentalpha;
			read_pixelxy_ppm(original->alpha, j, i, &currentalpha, original->height, original->width);
			write_block(out->alpha, j*factor, i*factor, factor, currentalpha, out->height, out->width);
		}
	}
	return out;
}

unsigned short get_block_alpha_float(unsigned short* alpha,double j, double i,int height,int width,int factor)
{
	unsigned short out;
	unsigned long totalalpha=0;
	unsigned short pixelvalue=0;
	size_t count=0;
	int x;
	int y;
	for( y = i; y < i + factor; y++)
	{
		for(x = j; x < j + factor; x++)
		{
			if(read_pixelxy_ppm(alpha, x, y, &pixelvalue, height, width) == 0)
			{
				count++;
				totalalpha += pixelvalue;
			}
		}
	}
	totalalpha /= count;
	out = totalalpha;
	return out;
}
unsigned short get_block_color_float(unsigned short* color,double j, double i,int height,int width,int factor)
{
	unsigned short out;
	unsigned long totalred=0;
	unsigned long totalgreen=0;
	unsigned long totalblue = 0;
	unsigned short pixelvalue=0;
	size_t count=0;
	int x;
	int y;
	printf("%d, %d", j, i);
	for( y = i; y < i + factor; y++)
	{
		for(x = j; x < j + factor; x++)
		{
			printf("ok1.125");
			if(read_pixelxy_ppm(color, x, y, &pixelvalue, height, width) == 0)
			{
				printf("ok1.25\n");
				count++;
				totalred += GET_R(pixelvalue);
				totalgreen += GET_G(pixelvalue);
				totalblue += GET_B(pixelvalue);
			}
		}
	}
	printf("ok1.5\n");
	if(count == 0)
		out = 0;
	else{
	totalred /= count;
	totalgreen /= count;
	totalblue /= count;
	out = RGB(totalred,	totalgreen,totalblue);
	}
	return out;
}

ppm_t* reduce_ppm_no_fill(ppm_t* original, double factor)
{
	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
	out->height = original->height/factor +.5;
	out->width = original->width/factor+.5;
	out->color = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	out->alpha = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	return out;
}


void fill_reduced_ppm(ppm_t * original, ppm_t* out)
{
	double factor = (double)original->height/out->height;
	size_t i;
	size_t j;
	int iratio = (int)((original->width<<16)/out->width) +1;
	unsigned short currentcolor;
	unsigned short currentalpha;
	int accessj;
	int accessi;
	for(i=0; i < out->height; i++)
		{
			for(j = 0; j < out->width; j++)
			{
				accessj=  (j*iratio)>> 16;
				accessi = (i*iratio)>> 16;
				currentcolor = get_block_color(original->color, j*factor, i*factor, original->height, original->width, factor);
				unchecked_write_pixelxy_ppm(out->color, j, i, currentcolor, out->height, out->width);
				currentalpha = get_block_alpha(original->alpha, j*factor, i*factor, original->height, original->width, factor);
				unchecked_write_pixelxy_ppm(out->alpha, j, i,currentalpha, out->height, out->width);
			}
		}
}

ppm_t* reduce_float(ppm_t * original, double factor)
{
	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
	out->height = original->height/factor +.5;
	out->width = original->width/factor+.5;
	out->color = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	out->alpha = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	size_t i;
	size_t j;
	int iratio = (int)((original->width<<16)/out->width) +1;
	unsigned short currentcolor;
	unsigned short currentalpha;
	int accessj;
	int accessi;
	for(i=0; i < out->height; i++)
		{
			for(j = 0; j < out->width; j++)
			{
				accessj=  (j*iratio)>> 16;
				accessi = (i*iratio)>> 16;
				currentcolor = get_block_color(original->color, j*factor, i*factor, original->height, original->width, factor);
				unchecked_write_pixelxy_ppm(out->color, j, i, currentcolor, out->height, out->width);
				currentalpha = get_block_alpha(original->alpha, j*factor, i*factor, original->height, original->width, factor);
				unchecked_write_pixelxy_ppm(out->alpha, j, i,currentalpha, out->height, out->width);
			}
		}
	return out;
}
unsigned short get_block_color_o(unsigned short* color,int j, int i,int height,int width,int factor,int pix_pow_2)
{
	unsigned short out;
	unsigned long totalred=0;
	unsigned long totalgreen=0;
	unsigned long totalblue=0;
	unsigned short pixelvalue=0;
	size_t count=0;
	size_t numpixels = 1 << pix_pow_2;
	size_t countx = 0;
	int x=j;
	int y;
	int step = ((int)(factor*factor))/numpixels;
	for( y = i; y < i + factor; y++)
	{
		for(x=j; 1; x +=step)
		{
			if(count >= numpixels)
				break;
			if(read_pixelxy_ppm(color, x, y, &pixelvalue, height, width) == 0)
			{
				count++;
				totalred += GET_R(pixelvalue);
				totalgreen += GET_G(pixelvalue);
				totalblue += GET_B(pixelvalue);
			}
			if(x > factor+j)
			{
				x = (x%factor) + j;
				break;
			}

		}
	}
	totalred >>= pix_pow_2;
	totalgreen >>= pix_pow_2;
	totalblue >>= pix_pow_2;
	out = RGB(totalred,	totalgreen,totalblue);
	return out;
}
unsigned short get_block_alpha_o(unsigned short* alpha,int j, int i,int height,int width,int factor,int pix_pow_2)
{
	unsigned short out;
	unsigned long totalalpha=0;
	unsigned short pixelvalue=0;
	size_t count=0;
	size_t numpixels = 1 << pix_pow_2;
	size_t countx = 0;
	int x=j;
	int y;
	int step = ((int)(factor*factor))/numpixels;
	for( y = i; y < i + factor; y++)
	{
		for(x=j; 1; x +=step)
		{
			if(count >= numpixels)
				break;
			if(read_pixelxy_ppm(alpha, x, y, &pixelvalue, height, width) == 0)
			{
				count++;
				totalalpha += pixelvalue;
			}
			if(x > factor+j)
			{
				x = (x%factor) + j;
				break;
			}
		}
	}
	totalalpha >>= pix_pow_2;
	out = totalalpha;
	return out;
}

ppm_t* fill_reduced_ppm_o(ppm_t * original,ppm_t* out)
{
	double factor = (double)original->height/out->height;
	size_t i;
	size_t j;
	int iratio = (int)((original->width<<16)/out->width) +1;
	unsigned short currentcolor;
	unsigned short currentalpha;
	int accessj;
	int accessi;
	int numpixels =(int)(factor*factor);
	int pix_pow_2 = 0;
	while(numpixels > 1)
	{
		numpixels >>= 1;
		pix_pow_2++;
	}
	for(i=0; i < out->height; i++)
		{
			for(j = 0; j < out->width; j++)
			{
				accessj=  (j*iratio)>> 16;
				accessi = (i*iratio)>> 16;
				currentcolor = get_block_color_o(original->color, j*factor, i*factor, original->height, original->width, (int)factor,pix_pow_2);
				unchecked_write_pixelxy_ppm(out->color, j, i, currentcolor, out->height, out->width);
				currentalpha = get_block_alpha_o(original->alpha, j*factor, i*factor, original->height, original->width, (int)factor, pix_pow_2);
				unchecked_write_pixelxy_ppm(out->alpha, j, i,currentalpha, out->height, out->width);
			}
		}
	return out;
}
//ppm_t* reduce_float2(ppm_t * original, int h2, int w2)
//{
//	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
//	out->height = h2;
//	out->width = w2;
//	out->color = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
//	out->alpha = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
//
//	int x_ratio = (int)((original->width<<16)/out->width) +1;
//	int y_ratio = (int)((original->height<<16)/out->height) +1;
//	int x2, y2 ;
//	int i, j;
//	for (i=0;i<out->height;i++) {
//		for (j=0;j<out->width;j++) {
//			x2 = ((j*x_ratio)>>16);
//			y2 = ((i*y_ratio)>>16);
//			out->color[(i*out->width)+j] = original->color[(y2*original->width)+x2] ;
//			out->alpha[(i*out->width)+j] = original->alpha[(y2*original->width)+x2] ;
//		}
//	}
//	return out ;
//}

ppm_t* rotate_ppm(ppm_t* original, int angle)
{
	angle = angle % 360;
	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
	out->width = (int)(sqrt((double)(original->width*original->width + original->height*original->height))+.5);
	out->height = out->width;
	out->color = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	out->alpha = (unsigned short*) malloc(out->width*out->height*sizeof(unsigned short));
	memset(out->alpha,0,out->width*out->height*sizeof(unsigned short));
	int y;
	int x;
	double o = (double)angle*PI/180;
	double sino = sin(o);
	double coso= cos(o);
	double x0 = (double)original->width/2;
	double y0 = (double)original->height/2;
	for(y = 0; y < out->height; y++)
	{
		for(x=0; x < out->width ; x++)
		{
			unsigned short currentcolor;
			unsigned short currentalpha;
			double x2= x;
			double y2 = y;
			double x1 = sino*y2 - sino*y0 + coso*x2 + (1 - coso)*x0;
			double y1 = coso*y2 + (1 - coso)*y0 - sino*x2 + sino*x0;
			if(read_pixelxy_ppm(original->color, x1, y1, &currentcolor, original->height, original->width) ==0)
				unchecked_write_pixelxy_ppm(out->color, x, y, currentcolor, out->height, out->width);
			if(read_pixelxy_ppm(original->alpha, x1, y1, &currentalpha, original->height, original->width)==0)
				unchecked_write_pixelxy_ppm(out->alpha, x, y, currentalpha, out->height, out->width);
		}
	}
	return out;
}

int compressalpha(unsigned char* uncompressed, unsigned short* target, size_t numpixels)
{
	size_t i = 0;
	long unsigned alpha;
	for(; i < numpixels; i++)
	{
		alpha = uncompressed[0];
		alpha= alpha * MAX_ALPHA /255;
		*target = (unsigned short) alpha;//
		target++;
		uncompressed+= 6;
	}
	return 0;
}
int compresscolor(unsigned char* uncompressed, unsigned short* target,size_t numpixels)
{
	size_t i = 0;
	long unsigned red, green, blue;
	for(; i < numpixels; i++)
	{
		red = uncompressed[0];
		green = uncompressed[2];
		blue = uncompressed[4];
		red = MAX_RED*red/255;
		green = MAX_GREEN*green/255;
		blue = MAX_BLUE*blue/255;
		*target = (unsigned short) RGB(red, green, blue);//
		target++;
		uncompressed+= 6;
	}
	return 0;
}

unsigned short* read_ppm_map(char* file, int* w, int* h, char iscolor)
{
	FILE *color;
	color = fopen(file, "r");
	if(color == NULL)
	{
		printf("Could not open %s\n", file);
		return NULL;
	}
	char identifier[3] ;
	if(fgets(identifier, 3, color) == NULL)
	{
		printf("Error reading file\n");
		fclose(color);
		return NULL;
	}
	int colorheight, colorwidth;
	if((fscanf(color, "%d", &colorheight) == EOF) || (fscanf(color, "%d", &colorwidth) == EOF))
	{
		printf("%d,%d\n",colorheight, colorwidth);
		printf("Error reading file1\n");
		fclose(color);
		return NULL;
	}
	*h = colorheight;
	*w = colorwidth;
	unsigned short* out = (unsigned short*) malloc(colorheight*colorwidth*sizeof(short));
	if(out == NULL)
	{
		printf("Could not allocate memory\n");
		fclose(color);
		return NULL;
	}
	int maxcolor;
	if(fscanf(color, "%d", &maxcolor) == EOF || fscanf(color, "%d", &maxcolor) == EOF)
	{
		printf("Error reading file3\n");
		free(out);
		fclose(color);
		return NULL;
	}
	unsigned char* uncompressed = (unsigned char*)malloc( colorheight*colorwidth * 6);
	if(uncompressed == NULL)
	{
		printf("Could not allocate memory\n");
		free(out);
		fclose(color);
		return NULL;
	}
	fread(uncompressed, sizeof(char), colorheight*colorwidth * 6, color);
	if(iscolor)
	compresscolor(uncompressed, out, colorheight*colorwidth);
	else compressalpha(uncompressed, out, colorheight*colorwidth);
	free (uncompressed);
	return out;
}

ppm_t* read_ppm(char* colorfile, char* alphafile)
{
	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
	if(out == NULL)
	{
		printf("Could not allocate memory\n");
		return NULL;
	}
	int colorw, colorh;
	if((out->color= read_ppm_map(colorfile,&colorw, &colorh,1)) == NULL)
	{
		printf("Could not read file\n");
		delete_ppm_t(out);
		return NULL;
	}
	int alphaw, alphah;
	if((out->alpha= read_ppm_map(alphafile,&alphaw, &alphah,0)) == NULL)
	{
		printf("Could not read file\n");
		delete_ppm_t(out);
		return NULL;
	}
	if(alphaw != colorw || colorh!= alphah)
	{
		printf("Error reading file, sizes dont match\n");
		delete_ppm_t(out);
		return NULL;
	}
	out->width = colorw; out->height =colorh;
	return out;
}

ppm_t* read_ppm_noAlpha(char* colorfile)
{
	ppm_t* out = (ppm_t*) malloc(sizeof(ppm_t));
	if(out == NULL)
	{
		printf("Could not allocate memory\n");
		return NULL;
	}
	int colorw, colorh;
	if((out->color= read_ppm_map(colorfile,&colorw, &colorh,1)) == NULL)
	{
		printf("Could not read file\n");
		delete_ppm_t(out);
		return NULL;
	}
	out->alpha = NULL;
	out->width = colorw; out->height =colorh;
	return out;
}


