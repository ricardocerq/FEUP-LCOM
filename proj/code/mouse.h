#ifndef MOUSE_H
#define MOUSE_H

#include "ps2mouse.h"
#include "vector.h"
#include <minix/drivers.h>

/**
 * @defgroup mouse mouse
 * @{
 *
 * implementation of mouse functionality
 */

///@brief copy from one mouse state to another
///@param m1 mouse state 1
///@param m2 mouse state 2
void assign_mouse(mouse_state_t* m1, mouse_state_t* m2);

///@brief
///@return
mouse_state_t* get_mouse_state(void);

///@brief display all packets
void dump_packets(void);

///@brief updates mouse state
static void update_mouse_state(void);

///checks if mouse is updated since it was last accessed
///@return mouse is updated
int mouse_is_updated(void);

///@brief mouse interrupt handler
///@return zero on success, non-zero otherwise
int mouse_int_handler(void);

///@brief clears OUT_BUF
///@return zero on success, non-zero otherwise
int clear_buf();

///initialize mouse state
///@param m mouse state to initialize
void initMouseState(mouse_state_t* m);

///subscribe mouse interrupts
///@return mask containing irq_set on success else negative value
int mouse_subscribe_int(void);

///unsubscribe mouse interrupts
///@return zero on success, non-zero otherwise
int mouse_unsubscribe_int(void);

///get mouse delta
///@param packet
///@return mouse delta
coord_t getMousexy(short * packet);

///@brief display the packets on the screen
///@param packet
void display_packet(short * packet);

///@brief process the packets, displaying them
///@param packet pointer to mouse packets
///@param packetindex which packet will be prccessed next
///@param packetcount number of packets received
///@return zero on success, non-zero otherwise
int processpacket(short* packet, size_t* packetindex, unsigned short* packetcount);

///@brief process the packets without displaying them
///@param packet pointer to mouse packets
///@param packetindex which packet will be prccessed next
///@param packetcount number of packets received
///@return zero on success, non-zero otherwise
int processpacket_no_display(short* packet, size_t* packetindex, unsigned short* packetcount);

///@brief write a value to kbc considering RESEND as ERROR
///@param port port to send
///@param value value to write
///@param read_response indicates whether the response should be read
///@return zero on success, non-zero otherwise
int write_to_kbc_no_resend(port_t port,unsigned long value, unsigned char read_response);

///write a value to the mouse
///@param value to write
///@return zero on success, non-zero otherwise
int write_to_mouse(unsigned long value);

///read value from mouse
///@param data return value
///@return zero on success, non-zero otherwise
int read_mouse_value(unsigned long* data);

///@brief issue a command to the mouse
///@param cmd command to send
///@param arg argument to the mouse command if it has one
///@param hasargs indicates whether to send the argument
///@return zero on success, non-zero otherwise
int issue_mouse_command(unsigned long cmd, unsigned long arg, char hasargs);

///@brief display mouse status
///@param status of mouse
void display_status(unsigned short* status);

///@brief set a coordinate to (0,0)
///@param c coordinate to reset
void resetCoord(coord_t * c);

/** @} end of mouse */

#endif
