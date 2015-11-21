#ifndef UART_H
#define UART_H


#define BIT(n) (0x01<<(n))
#define SEP_BIT(num, n) ((BIT(n)&num)>>n) //returns bit n of num
#define SEP_BITS(num, a, b) ((num & (( (1<<(b-a+1)) - 1)<<a)) >> a) //returns bits a to b of num
#define WAIT_MS(ms) tickdelay(micros_to_ticks(ms*1000))
#define WAIT_TIME 50


#define ORIGINAL_SER_COM1_HOOK_ID 4
#define ORIGINAL_SER_COM2_HOOK_ID 3


#define COM2_IRQ 3
#define COM1_IRQ 4
#define UART_FREQ 115200

#define STD_RATE_VALUE_1 1200
#define STD_RATE_VALUE_2 2400
#define STD_RATE_VALUE_3 4800
#define STD_RATE_VALUE_4 9600
#define STD_RATE_VALUE_5 19200
#define STD_RATE_VALUE_6 38400
#define STD_RATE_VALUE_7 57600
#define STD_RATE_VALUE_8 115200

/*port adresses*/
#define COM1 0x3F8
#define COM2 0x2F8

/*registers*/

#define RECEIVER_BUF_REG 0
#define TRANSMITTER_HOLDING_REG 0
#define DIVISOR_LATCH_LSB 0
#define INT_ENABLE_REG 1
#define DIVISOR_LATCH_MSB 1
#define INT_ID_REG 2
#define FIFO_CTRL_REG 2
#define LINE_CTRL_REG 3
#define MODEM_CTRL_REG 4
#define LINE_STATUS_REG 5
#define MODEM_STATUS_REG 6
#define SCRATCHPAD_REG 7



/*Line Control Register*/
#define WORD_LENGTH_SELECT (BIT(1)|BIT(0))
#define NO_STOP_BITS BIT(2)
#define UART_PARITY (BIT(5)|BIT(4)|BIT(3))
#define SET_BREAK_ENABLE BIT(6)
#define DLAB BIT(7)

#define WORD_LEN_5 0
#define WORD_LEN_6 1
#define WORD_LEN_7 2
#define WORD_LEN_8 3

#define STOP_BITS_1 (0 << 2)
#define STOP_BITS_2 (1 << 2)

#define PARITY_ODD (1 << 3)
#define PARITY_EVEN (3 << 3)
#define PARITY_1 (5 << 3)
#define PARITY_0 (7 << 3)


/*Line Status Register*/
#define RECEIVER_READY BIT(0)
#define OVERRUN_ERROR BIT(1)
#define PARITY_ERROR_UART BIT(2)
#define FRAMING_ERROR BIT(3)
#define BREAK_INT BIT(4)
#define TRANSMITTER_HOLDING_REG_EMPTY BIT(5)
#define TRANSMITTER_EMPTY BIT(6)

/*Interrupt enable register*/
#define ENABLE_RECEIVED_DATA_INT BIT(0)
#define ENABLE_TRANSMITTER_EMPTY_INT BIT(1)
#define ENABLE_RECEIVER_LINE_STATUS_INT BIT(2)
#define ENABLE_MODEM_STATUS_INTERRUPT BIT(3)
#define IRRELEVANT_UART (BIT(5)|BIT(4))
#define RESERVED_INT_ENABLE_UART (BIT(7)|BIT(6))

/*Interrupt Identification Register*/
#define INT_STATUS  BIT(0)
#define INT_ORIGIN (BIT(2)|BIT(1))
#define RESERVED_INT_ID_UART BIT(4)
#define FIFO_64_BYTE BIT(5)
#define FIFO_STATUS (BIT(7)|BIT(6))

/*Interrupt origins*/
#define MODEM_STATUS_INT 0
#define	TRANSMITTER_EMPTY_INT (1 << 1)
#define	CHAR_TIMEOUT_INT (4 << 1)
#define	RECEIVED_DATA_AVAILABLE_INT (2 << 1)
#define	LINE_STATUS_INT (3 << 1)

/*FIFO Control Register*/
#define ENABLE_FIFO BIT(0)
#define CLEAR_RECEIVE_FIFO BIT(1)
#define CLEAR_TRANSMIT_FIFO BIT(2)
#define DMA_MODE_SELECT BIT(3)
#define RESERVED_FIFO_CTRL BIT(4)
#define ENABLE_64_BYTE_FIFO BIT(5)
#define FIFO_INT_TRIGGER_LEVEL BIT(6)


/*FIFO INITIALIZER*/
#define FIFO_INIT 0x87 // set trigger level to 8, clear fifos, enable fifo

/*COMMUNICATIONS PROTOCOL*/
typedef enum {TYPE_INT= 0x30, TYPE_SHORT, TYPE_CHAR, TYPE_STRING, TYPE_MESSAGE, TYPE_DOUBLE, TYPE_FLOAT} signal_type_t;

typedef enum {MES_HELLO =0x50,MES_ACK, MES_NACK, MES_BYE, MES_READY, MES_UNREADY, MES_WON, MES_LOST} message_type_t;


/** @} end of uart */

#endif
