#ifndef __KBD_H
#define __KBD_H

#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <minix/sysutil.h>

int kbd_subscribe_int(int block_kbd);

int kbd_unsubscribe_int(void);

int write_to_kbd(port_t port,unsigned long value);

int issue_kbd_command(port_t port,unsigned long cmd, unsigned long arg, char hasargs);

int read_kbd_value(unsigned long* data);

int kbd_scan_internal(int (*ih_wrapper)(unsigned long *,unsigned long *));

int kbd_test_leds_internal(unsigned short n, unsigned short *leds);

int kbd_test_timed_scan_internal(unsigned short seconds);

int scan_on_kbd_int_c(unsigned long* ticks,unsigned long* scancode);

extern int scan_on_kbd_int_ass(unsigned long* ticks,unsigned long *scancode);


//interrupt loop for any function that uses timer interrupts, kbd interrupts or both
int generic_interrupt_loop(unsigned long* var_timer, unsigned long* var_kbd, int (*on_timer_int)(unsigned long *,unsigned long *), int (*on_kbd_int)(unsigned long *,unsigned long *), char block_kbd, char (*timer_condition)(unsigned long,unsigned long), char (*kbd_condition)(unsigned long,unsigned long));


#endif
