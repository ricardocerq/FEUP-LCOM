#ifndef SER_PORT_H
#define SER_PORT_H

#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <minix/sysutil.h>
#include "uart.h"
#include "char_queue.h"

/**
 * @defgroup ser_port ser_port
 * @{
 *
 * implementation of UART functionality
 */

#include "ser_port.h"

///@brief get the size of reception queue
///@return size of reception queue
size_t get_receive_fifo_size();

///@brief get the size of transmission queue
///@return size of transmission queue
size_t get_transmit_fifo_size();

///@brief empty the reception queue
void empty_receive_fifo();

///@brief get the divisor of rate
///@param com_address port to change
///@param dl return value
///@return zero on success, non-zero otherwise
int get_divisor_latch(unsigned long com_address,unsigned long* dl);

///@brief set the divisor of rate
///@param com_address port
///@param dl value to write
///@return zero on success, non-zero otherwise
int set_divisor_latch(unsigned long com_address,unsigned long dl);

///@brief display the configuration of a port
///@param base_addr port
///@return zero on success, non-zero otherwise
int ser_conf(unsigned short base_addr);

///@brief change communication parameters
///@param base_addr port
///@param bits num of bits per character
///@param stop number of stop bits
///@param parity parity used
///@param rate rate of transmission
///@return zero on success, non-zero otherwise
int ser_set(unsigned short base_addr, unsigned long bits, unsigned long stop,
        long parity, unsigned long rate);

///@brief enable reception interrupts
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_set_receive_int_bit(unsigned long ser_port);

///@brief disable reception interrupts
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_clear_receive_int_bit(unsigned long ser_port);

///@brief enable transmission interrupts
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_set_transmit_int_bit(unsigned long ser_port);

///@brief disable transmission interrupts
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_clear_transmit_int_bit(unsigned long ser_port);

///@brief enable line status interrupts
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_set_line_int_bit(unsigned long ser_port);

///@brief disable line status interrupts
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_clear_line_int_bit(unsigned long ser_port);

///@brief subscribe com1 interrupts
///@return irq_set on success else negative value
int ser_subscribe_com1_int(void);

///@brief unsubscribe com1 interrupts
///@return zero on success, non-zero otherwise
int ser_unsubscribe_com1_int(void);

///@brief subscribe com2 interrupts
///@return irq_set on success else negative value
int ser_subscribe_com2_int(void);

///@brief unsubscribe com1 interrupts
///@return zero on success, non-zero otherwise
int ser_unsubscribe_com2_int(void);

///@brief initialize UART and sofware fifos
///@return zero on success, non-zero otherwise
int initialize_fifos(void);

///@brief subscribe interrupts for a certain port, for a certain role
///@param ser_port port
///@param tx role 1 for transmission, 0 for reception
///@return irq_set on success else negative value
int generic_ser_subscribe_int(unsigned long ser_port, unsigned char tx);

///@brief subscribe interrupts for a certain port, for a certain role
///@param ser_port port
///@param tx role 1 for transmission, 0 for reception
///@return zero on success, non-zero otherwise
int generic_ser_unsubscribe_int(unsigned long ser_port, unsigned char tx);

///@brief subscribe interrupts for a certain port
///@param ser_port port
///@return irq_set on success else negative value
int generic_ser_subscribe_int_tx_rx(unsigned long ser_port);

///@brief subscribe interrupts for a certain port
///@param ser_port port
///@return zero on success, non-zero otherwise
int generic_ser_unsubscribe_int_tx_rx(unsigned long ser_port);

///@brief indicates whether UART can receive more bytes
///@param com_address port
///@return zero on success, non-zero otherwise
int receiver_is_ready(unsigned long com_address);

///@brief indicates whether UART can hold more bytes for transmission
///@param com_address port
///@return zero on success, non-zero otherwise
int transmitter_is_ready(unsigned long com_address);

///@brief serial port interrupt handler
///@param ser_port port
///@return zero on success, non-zero otherwise
int ser_ih(unsigned long ser_port);

///@brief transmit a single byte
///@param byte information to transmit
///@return zero on success, non-zero otherwise
int transmit_b(unsigned char byte);

///@brief transmit multiple bytes
///@param data data to transmit
///@param size number of bytes of information
///@return zero on success, non-zero otherwise
int transmit_m(void * data, size_t size);

///@brief receive a single byte
///@param received byte return value
///@return zero on success, non-zero otherwise
int receive_b(unsigned char* received);

///@brief receive multiple bytes
///@param data return value
///@param size number of bytes
///@return zero on success, non-zero otherwise
int receive_m(void * data, size_t size);

///@brief executes the defined reception protocol
///@param data returned data
///@param signal_type returned type of signal
///@param size number of bytes received
///@return 0 if data was successfuly received
int receive_protocol(void* data, signal_type_t* signal_type, size_t* size);

///@brief executes the defined transmission protocol
///@param data data to transmit
///@param signal_type type of signal
///@param size number of bytes of data
///@return zero on success, non-zero otherwise
int transmit_protocol(void* data, signal_type_t signal_type, size_t size);

///@brief transmit a message according to the defined transmission protocol
///@param message message to send
///@return zero on success, non-zero otherwise
int transmit_message(message_type_t message);

/** @} end of cahr_queue */

#endif
