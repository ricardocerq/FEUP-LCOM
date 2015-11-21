#include <minix/drivers.h>
#include "ppm.h"
#include "game.h"
#include "rtc.h"
#include "mouse.h"
#include "menu.h"
#include "program.h"
#include "ser_port.h"
#include "uart.h"
#include <unistd.h>
static ppm_t* background = NULL;
static char* video_mem = NULL;
static char* secondary_buf= NULL;
static ppm_t* cell_ppms[CELL_PPM_COUNT];
static char cell_ppms_filled[CELL_PPM_COUNT];
static ppm_t*  original = NULL;
static cursor_t cursor;
static mouse_state_t mouse;
static mouse_state_t previous_mouse;
static char* date_str = NULL;
static int mouse_irq_set = 0;
static int kbd_irq_set = 0;
static int timer_irq_set = 0;
static int rtc_irq_set = 0;
static int com1_irq_set = 0;
static int com2_irq_set = 0;
static char * playername = NULL;
static unsigned long programinitialtime;
static char* player2name = NULL;
static unsigned long player2time = 0;
static role_t role = SEARCHING;
static char* dir = NULL;
static char* dir_temp = NULL;
void reset_video_mode()
{
	vg_init(PROGRAM_VIDEO_MODE);
	vg_exit();
}

int io_init()
{
	int failure = 0;
	if((video_mem=(char *)vg_init(PROGRAM_VIDEO_MODE)) == NULL)
	{
		printf("test_move(): vg_init() failed");
		return 1;
	}
	secondary_buf = (char*) get_secondary_buf();
	if((kbd_irq_set = kbd_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("project(): kbd_subscribe_int() failed \n");
		failure = 1;
	}
	if((timer_irq_set = timer_subscribe_int()) < 0){//subscribe timer 0 interrupts
		printf("project(): timer_subscribe_int() failed \n");
		failure = 1;
	}
	if((mouse_irq_set = mouse_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("project(): mouse_subscribe_int() failed \n");
		failure = 1;
	}
	if((rtc_irq_set = rtc_subscribe_int()) < 0){//subscribe kbd interrupts
		printf("test_int(): rtc_subscribe_int() failed \n");
		failure = 1;
	}
	if(enable_update_ended_int())
		failure = 1;
	if(ISSUE_MOUSE_CMD_NO_ARGS(SET_STREAM_MODE))
	{
		printf("Failure setting stream mode\n");
		failure = 1;
	}
	if(ISSUE_MOUSE_CMD_NO_ARGS(ENABLE_DATA_PACKETS))
	{
		printf("Failure enabling data packets\n");
		failure = 1;
	}
	return failure;
}

int io_exit()
{
	int failure = 0;
	ISSUE_MOUSE_CMD_NO_ARGS(DISABLE_DATA_PACKETS);
	if(timer_unsubscribe_int()){//unsubscribe interrupts
		printf("project(): timer_unsubscribe_int() failed\n");
		failure = 1;
	}
	if(kbd_unsubscribe_int()){//unsubscribe interrupts
		printf("project(): kbd_unsubscribe_int() failed\n");
		failure = 1;
	}
	if(mouse_unsubscribe_int()){//unsubscribe interrupts
		printf("project(): mouse_unsubscribe_int() failed\n");
		failure = 1;
	}
	if(rtc_unsubscribe_int()){//unsubscribe interrupts
		printf("test_project(): rtc_unsubscribe_int() failed\n");
		failure = 1;
	}

	if(vg_exit()){
		printf("project(): vg_exit() failed");
		return 1;
	}
	if(write_rtc_data(playername, MAX_PLAYER_NAME_LENGTH))
		return 1;
	if(commit_rtc_data(PROGRAM_SIGNATURE))
		return 1;
	return failure;
}

int rtc_data_init()
{
	if(read_rtc_data())
		return 1;
	if(verify_rtc_data_source(PROGRAM_SIGNATURE))
	{
		char* buf =(char*)get_rtc_saved_data();
		size_t i = 0;
		for(i = 0; buf[i] != '\0' && i < MAX_PLAYER_NAME_LENGTH-1; i++)
		{}
		i++;
		playername = (char *) malloc(sizeof(char)* i);
		if(playername == NULL)
			return 1;
		for(i = 0;buf[i] != '\0'&& i < MAX_PLAYER_NAME_LENGTH-1; i++)
		{
			playername[i] = buf[i];
		}
		playername[i] = '\0';
		if(strlen(playername) == 0)
			return get_player_name();
		return 0;
	}
	else return get_player_name();
}


int initialize_program()
{
	programinitialtime = getTimeSeconds();
	srand(programinitialtime);
	if(initialize_font()) return 1;
	lm_init();
	background = (ppm_t *)read_ppm_noAlpha(get_dir("/ppms/background.ppm"));
	if(background == NULL)
		return 1;
	if((original = read_ppm(get_dir("/ppms/osmos.ppm"),get_dir("/ppms/osmosalpha.ppm"))) == NULL) return 1;
	memset(cell_ppms,0,sizeof(cell_t*)*CELL_PPM_COUNT);
	memset(cell_ppms_filled, 0, sizeof(char)*CELL_PPM_COUNT);
	allocate_ppm_space();
	cell_ppms[CELL_PPM_COUNT-1] = original;
	cell_ppms_filled[CELL_PPM_COUNT-1] = 1;
	getDateString(&date_str);
	if(initialize_cursor(&cursor)) return 1;
	if(io_init())
		return 1;
	if(rtc_data_init())
		return 1;
	return 0;
}

void free_program_resources()
{
	free_fonts();
	free_cell_ppms();
	delete_ppm_t(background);
	delete_ppm_t(cursor.pixmap);
}

int main(int argc, char **argv)
{
	sef_startup();
	sys_enable_iop(SELF);
	if(argc == 2)
	{
		if (strncmp(argv[1], "clear", strlen("clear")) == 0)
		{
			reset_video_mode();
			clear_buf();
			return 0;
		}
	}
	if(initialize_dir(argv[0]))
		return 0;
	printf("Loading...\n");
	if(initialize_program())
	{
		printf("initialize program failed\n");
		return 0;
	}
	main_menu();
	if(io_exit()){
		printf("io_exit() failed\n");
	}
	free_program_resources();
	clear_buf();
	return 0;
}

ppm_t* get_background(){
	return background;
};
char* get_program_video_mem()
{
	return video_mem;
}
char* get_program_secondary_buf()
{
	return secondary_buf;
}

int get_mouse_irq_set(){
	return mouse_irq_set;
};

 int get_kbd_irq_set(){
	 return kbd_irq_set;
 };


int get_timer_irq_set(){
	 return timer_irq_set;
 };

int get_rtc_irq_set(){
	return rtc_irq_set;
};

int get_com1_irq_set(){
	return com1_irq_set;
};

int set_com1_irq_set(int new_com1){
	return com1_irq_set = new_com1;
};

int get_com2_irq_set(){
	return com2_irq_set;
};

int set_com2_irq_set(int new_com2){
	return com1_irq_set = new_com2;
};

char* get_program_playername()
{
	return playername;
}

char* set_program_playername(char* new_p_n)
{
	if(playername != NULL)
		free(playername);
	return playername = new_p_n;
}

unsigned long* get_programinitialtime_ptr()
{
	return &programinitialtime;
}


char* get_program_playername2()
{
	return player2name;
}

char* set_program_player2name(char* new_p_n)
{
	if(player2name != NULL)
		free(player2name);
	return player2name = new_p_n;
}

unsigned long get_player2time()
{
	return player2time;
}

unsigned long set_player2time(unsigned long new_v)
{
	return player2time = new_v;
}

role_t set_role(role_t new_role)
{
	return role = new_role;
}

role_t get_role()
{
	return role;
}

char** get_date_str_ptr()
{
	return &date_str;
}

void initialize_single_cell_pixmap(int index)
{
	if(cell_ppms[index] == NULL)
		cell_ppms[index] = (ppm_t*) reduce_float(original, 250/((float)(index+1)/10));
	return;
}

void allocate_ppm_space()
{
	size_t i = 0;
	for(; i < CELL_PPM_COUNT - 1; i++)
	{
		cell_ppms[i] = (ppm_t*)reduce_ppm_no_fill(original, (double) 250/((double)(i+1)/10));
	}
}

ppm_t* get_cell_pixmap(float radius)
{
	int index = radius * 10 ;
	index--;
	if(index < 0)
		index = 0;
	if(index > CELL_PPM_COUNT - 1)
		index = CELL_PPM_COUNT-1;
	if(cell_ppms_filled[index] == 0)
	{
		fill_reduced_ppm_o(original, cell_ppms[index]);
		cell_ppms_filled[index] = 1;
	}
	return cell_ppms[index];
}

void free_cell_ppms()
{
	size_t i = 0;
	for(; i < CELL_PPM_COUNT; i++)
	{
		delete_ppm_t(cell_ppms[i]);
	}
}

cursor_t* get_cursor()
{
	return &cursor;
}

mouse_state_t* get_mouse()
{
	return &mouse;
}

mouse_state_t* get_previous_mouse()
{
	return &previous_mouse;
}

char* get_dir(char * file)
{
	strcpy(dir_temp, dir);
	strcat(dir_temp, file);
	char* out = (char*)malloc(DIR_MAX*sizeof(char));
	strcpy(out, dir_temp);
	return out;
}

int initialize_dir(char* path)
{
	char temp_path[DIR_MAX];
	strcpy(temp_path, path);
//	char* result = 	strstr(temp_path, "project");
//	if(result == NULL)
//		return 1;
	char* result = temp_path;
	while(*result != '\0')
	{result++;}
	while(*result != '/')
	{result--;}
	*result= '\0';
    dir = (char *)malloc(sizeof(char)*(strlen(temp_path)+1));
    strcpy(dir, temp_path);
    dir_temp = (char *)malloc(sizeof(char)*DIR_MAX);
    return 0;
}
