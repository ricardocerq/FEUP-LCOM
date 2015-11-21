#ifndef __I8042_H
#define __I8042_H

#define MAX_ATTEMPTS 10
#define ASS_NUM_LOOPS 5600000
#define WAIT_MS(ms) tickdelay(micros_to_ticks(ms*1000))
#define WAIT_TIME 50
#define BIT(n) (0x01<<(n))
#define SEP_BIT(num, n) (BIT(n)&num)>>n //returns bit n of num
#define SEP_BITS(num, a, b) (num & (( (1<<(b-a+1)) - 1)<<a)) >> a //returns bits a to b of num
#define TOGGLE_BIT(num, bit) num = num^BIT(bit)

#define BREAK_CODE_BIT BIT(7)
#define TWO_BYTE 0xE0
#define LEDS_CMD 0xED

#define ORIGINAL_KBD_HOOK_ID 1
#define KBD_IRQ 1


/* Keyboard controller status register */

#define PARITY_ERROR BIT(7)
#define TIMEOUT BIT(6)
#define AUX_OUT_BUF_FULL BIT(5)
#define KEYBOARD_LOCK BIT(4)
#define COMMAND_DATA BIT(3)
#define SYSTEM_FLAG BIT(2)
#define IN_BUF_STATUS BIT(1)
#define OUT_BUF_STATUS BIT(0)

/* Keyboard controller command byte */

#define UNUSED_KBD BIT(7)
#define TRANLATE BIT(6)
#define MOUSE_ENABLE BIT(5)
#define KEYBOARD_ENABLE BIT(4)
#define IGNORE_KEYBOARD_LOCK BIT(3)
#define SYSTEM_FLAG BIT(2)
#define MOUSE_INT_ENABLE BIT(1)
#define KEYBOARD_INT_ENABLE BIT(0)

/* Keyboard controller command byte */

#define READ_KBD_CTRL_CMD_BYTE 0x20
#define WRITE_KBD_CTRL_CMD_BYTE 0x60
#define SELF_TEST 0xaa
#define INTERFACE_TEST 0xab
#define DISABLE_KBD_KBC 0xad
#define ENABLE_KBD_KBC 0xae
#define READ_IN_PORT 0xc0
#define READ_OUT_PORT 0xd0
#define WRITE_OUT_PORT 0xd1
#define READ_TEST_INPUTS 0xe0
#define SYSTEM_RESET 0xfe


/* I/O port addresses */
#define OUT_BUF 0x60
#define IN_BUF 0x60
#define KBC_CMD_REG 0x64
#define STAT_REG 0x64

 /* KBD responses */

#define RESEND 0xFE
#define ERROR 0xFC
#define ACK 0xFA

#define ESC_BREAK 0x81

#endif
