#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "vbe.h"
#include "lmlib.h"
#include "graphics_mode.h"



#define PB2BASE(x) (((x) >> 4) & 0x0F000)
#define PB2OFF(x) ((x) & 0x0FFFF)

int vbe_get_mode_info(unsigned short mode, vbe_mode_info_t *vmi_p) {
  
	mmap_t map;
	void* info;
	info = lm_alloc(sizeof(vbe_mode_info_t), &map);
	if(info == NULL)
		{
			printf("vbe_get_mode_info(): lm_alloc failed\n");
			return 1;
		}
	struct reg86u reg;
	reg.u.w.ax = (VBE_IDENTIFIER<<8)|(VBE_F_MODE_INFO);
	reg.u.w.cx = mode;
	reg.u.w.es = PB2BASE(map.phys);
	reg.u.w.di = PB2OFF(map.phys);
	printf("map.phys: %x\n", map.phys);
	reg.u.b.intno = VIDEO_CARD_INT;
	if( sys_int86(&reg) != OK ) {
		printf("vbe_get_mode_info(): sys_int86() failed \n");
		lm_free(&map);
		return 1;
	}
	if(reg.u.b.ah == VBE_FAIL)
	{
		printf("vbe_get_mode_info(): Function call failed\n");
		lm_free(&map);
		return 1;
	}
	else if(reg.u.b.ah == VBE_N_SUP)
	{
		printf("vbe_get_mode_info(): Function is not supported in current HW configuration\n");
		lm_free(&map);
		return 1;
	}
	else if(reg.u.b.ah == VBE_INVALID)
	{
		printf("vbe_get_mode_info(): Function is invalid in current video mode\n");
		lm_free(&map);
		return 1;
	}
	else if(reg.u.b.ah != VBE_OK)
	{
		printf("vbe_get_mode_info(): error");
		lm_free(&map);
		return 1;
	}
	*vmi_p= * (vbe_mode_info_t*)map.virtual;
	lm_free(&map);
    return 0;
}
int vbe_get_controller_info(vbe_controller_info_t *vmi_p, void** base) {
	mmap_t map;
	void* info;
	info = lm_alloc(sizeof(vbe_controller_info_t), &map);
	if(info == NULL)
	{
		printf("vbe_get_controller_info(): lm_alloc failed\n");
		return 1;
	}
	char* vbesig = "VBE2";
	memcpy(info, vbesig, 4); // set vbe signature to "VBE2"
	struct reg86u reg;
	reg.u.w.ax = (VBE_IDENTIFIER<<8)|(VBE_F_CTRLER_INFO);
	reg.u.w.es = PB2BASE(map.phys);
	reg.u.w.di = PB2OFF(map.phys);
	reg.u.b.intno = VIDEO_CARD_INT;
	if( sys_int86(&reg) != OK ) {
		printf("vbe_get_controller_info(): sys_int86() failed \n");
		lm_free(&map);
		return 1;
	}
	if(reg.u.b.ah == VBE_FAIL)
	{
		printf("vbe_get_controller_info(): Function call failed\n");
		lm_free(&map);
		return 1;
	}
	else if(reg.u.b.ah == VBE_N_SUP)
	{
		printf("vbe_get_controller_info(): Function is not supported in current HW configuration\n");
		lm_free(&map);
		return 1;
	}
	else if(reg.u.b.ah == VBE_INVALID)
	{
		printf("vbe_get_controller_info(): Function is invalid in current video mode\n");
		lm_free(&map);
		return 1;
	}
	else if(reg.u.b.ah != VBE_OK)
	{
		printf("vbe_get_controller_info(): error");
		lm_free(&map);
		return 1;
	}
	memcpy(vmi_p, map.virtual,sizeof(vbe_controller_info_t)); //copy from info to struct
	*base = (void*)GET_BASE((int)map.virtual); //return segment mapped by lm_init
	lm_free(&map);
    return 0;
}
//print bcd value as version
void print_vbe_version(unsigned long version)
{
	char str[4];
	sprintf(str, "%X.%X", (version >> 8),(version & 0xFF));
	printf("%s", str);
}


