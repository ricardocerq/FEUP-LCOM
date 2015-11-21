#ifndef MOUSE_H
#define MOUSE_H

#include "ps2mouse.h"
#include "vector.h"
#include <minix/drivers.h>

int mouse_subscribe_int(void);

int mouse_unsubscribe_int(void);

coord_t getMousexy(short * packet);

void display_packet(short * packet);

int processpacket(short* packet, size_t* packetindex, unsigned short* packetcount);

int write_to_kbc_no_resend(port_t port,unsigned long value, unsigned char read_response);

int write_to_mouse(unsigned long value);

int issue_mouse_command(unsigned long cmd, unsigned long arg, char hasargs);

void display_status(unsigned short* status);

void resetCoord(coord_t * c);

coord_t addCoord(coord_t c1, coord_t c2);

void printCoord(coord_t c);

void getEvent2(event_t* ev, mouse_state_t* m, short * packet);

int check_hor_line2(event_t* evt,short length, unsigned short tolerance, vector_t* trajectories);

#endif
