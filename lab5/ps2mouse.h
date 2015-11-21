#ifndef PS2_MOUSE_H
#define PS2_MOUSE_H

#define MOUSE_IRQ 12
#define ORIGINAL_MOUSE_HOOK_ID 2

#define WAIT_MS(ms) tickdelay(micros_to_ticks(ms*1000))
//#define WAIT_TIME 100
#define BIT(n) (0x01<<(n))
#define GET_BIT_NUM(num, bit) (((bit&num)== 0)? 0:1)
#define XOR(a,b) (a ? !b : b)
#define ISSUE_MOUSE_CMD_NO_ARGS(cmd) issue_mouse_command(cmd, 0, 0)
#define ISSUE_MOUSE_CMD_ARGS(cmd, arg) issue_mouse_command(cmd, arg, 1)

typedef enum   {INIT=0, DRAW, COMP} state_t;
typedef enum   {LUP=0, LDOWN, MOVE} ev_type_t;
typedef struct   {int x; int y;} coord_t;
typedef struct { coord_t delta;  ev_type_t type;} event_t;
typedef struct {coord_t coords; int lb; int mb; int rb;} mouse_state_t;



//byte 1


#define Y_OVF BIT(7)
#define X_OVF BIT(6)
#define Y_SIGN BIT(5)
#define X_SIGN BIT(4)
#define BYTE_1_BIT BIT(3)
#define M_B BIT(2)
#define R_B BIT(1)
#define L_B BIT(0)

#define IS_BYTE_1(num) num&BYTE_1_BIT

//mouse related kbc commands
#define DISABLE_MOUSE_KBC 0xA7
#define ENABLE_MOUSE 0xA8
#define CHECK_MOUSE_INTERFACE 0xA9
#define WRITE_BYTE_MOUSE 0xD4

//kbc command byte
#define DISABLE_MOUSE BIT(5)
#define DISABLE_KBD BIT(4)
#define INT2 BIT(1)
#define INT BIT(0)
//PS/2 mouse commands
#define RESET 0xFF
#define RESEND_CMD 0xFE
#define SET_DEFAULTS 0xF6
#define DISABLE_DATA_PACKETS 0xF5
#define ENABLE_DATA_PACKETS 0xF4
#define SET_SAMPLE_RATE 0xF3
#define SET_REMOTE_MODE 0xF0
#define READ_DATA 0xEB
#define SET_STREAM_MODE 0xEA
#define STATUS_REQUEST 0xE9
#define SET_RESOLUTION 0xE8
#define SET_SCALING_2_1 0xE7
#define SET_SCALING_1_1 0xE6

//status byte format
#define REMOTE_BIT BIT(6)
#define DATA_REPORTING_BIT BIT(5)
#define SCALING_BIT BIT(4)
#define RESOLUTION_BITS (BIT(1)|BIT(0))



#endif
