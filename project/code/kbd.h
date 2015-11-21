#ifndef __KBD_H
#define __KBD_H

#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <minix/sysutil.h>
#include "ps2mouse.h"

/**
 * @defgroup kbd kbd
 * @{
 *
 * implementation of keyboard functionality
 */

///@brief keyboard interrupt handler
///@return zero on success, non-zero otherwise
int kbd_int_handler(void);

///@brief get last scancode received
///@return last received keyboard scancode
unsigned long get_scancode(void);

///@brief checks if last scancode received is first byte of a two byte scancode
///@return 1 if first byte else 0
char is_first_byte(void);

///@brief subscribe keyboard interrups
///@return irq_set on success, negative otherwise
int kbd_subscribe_int();

///@brief unsubscribe keyboard interrupts
///@return zero on success, non-zero otherwise
int kbd_unsubscribe_int(void);

///@brief print a scancode
///@param scancode scancode to print
void print_scan_code(unsigned long scancode);

///writes a value to the keyboard
///@param port port to write to
///@param value value to write
///@return zero on success, no-zero otherwise
int write_to_kbd(port_t port,unsigned long value);

///@brief sends command with optional argument.
///@param port port to send to
///@param cmd command to send
///@param arg optional argument to command
///@param hasargs indicates whether argument should be written
///@return zero on success, no-zero otherwise
int issue_kbd_command(port_t port,unsigned long cmd, unsigned long arg, char hasargs);

///@brief reads a value from the keyboard
///@param data return value
///@return zero on success, no-zero otherwise
int read_kbd_value(unsigned long* data);

///@brief process input from the keyboard
///@param scancode scancode received
///@param character returned character, if scancode corresponds to one
///@return indeicate whether a character was received (CHAR_RECEIVED) or not (IGNORE_INPUT)
int process_input(unsigned long scancode, char* character);

/** @} end of kbd */

#endif
