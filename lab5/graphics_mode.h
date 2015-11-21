#ifndef GRAPHICS_MODE_H
#define GRAPHICS_MODE_H
//#include <machine/int86.h>

#define VIDEO_CARD_INT 0x10
#define VBE_IDENTIFIER 0x4F
//VBE return values
#define VBE_OK 0x00
#define VBE_FAIL 0x01	//Function call failed
#define VBE_N_SUP 0x02	//Function is not supported in current HW configuration
#define VBE_INVALID 0x03	//Function is invalid in current video mode

//VBE modes (to be passed in BX)

#define VBE_MODE_RES_640x480 	0x101
#define VBE_MODE_RES_800x600 	0x103
#define VBE_MODE_RES_1024x768 	0x105
#define VBE_MODE_RES_1280x1024 	0x107


//VBE functions

#define VBE_F_CTRLER_INFO 0x00
#define VBE_F_MODE_INFO 0x01
#define VBE_F_SET_MODE 0x02
#define VBE_F_CURRENT_VBE_MODE 0x03
#define VBE_F_DISPLAY_WINDOW_CTRL 0x05
#define VBE_F_LOGICAL_SCAN_LINE_LENGTH 0x06
#define VBE_DISPLAY_START 0x07
#define VBE_F_DAC_PALETTE_FORMAT 0x08
#define VBE_F_PALETTE_DATA 0x09
#define VBE_F_FUNCTION_INFO 0x0A




#endif
