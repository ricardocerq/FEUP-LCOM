#include "ser_port.h"


static int ser_com1_hook_id = ORIGINAL_SER_COM1_HOOK_ID;
static int ser_com2_hook_id = ORIGINAL_SER_COM2_HOOK_ID;
static char_queue_t* receive_fifo;
static char_queue_t* transmit_fifo;
static unsigned long transmit_port = 0;
static unsigned long receiver_port = 0;
static unsigned long received_char = 0;

size_t get_receive_fifo_size()
{
	return size_q(receive_fifo);
}
size_t get_transmit_fifo_size()
{
	return size_q(transmit_fifo);
}
void empty_receive_fifo()
{
	clear_q(receive_fifo);
}
int get_divisor_latch(unsigned long com_address,unsigned long* dl)
{
	unsigned long lsb = 0;
	unsigned long msb = 0;
	unsigned long lcr = 0;
	if(sys_inb(com_address + LINE_CTRL_REG, &lcr) != OK)
	{
		printf("Error reading LINE_CTRL_REG\n");
		return 1;
	}
	unsigned long newlcr = lcr | DLAB;
	if(sys_outb(com_address + LINE_CTRL_REG, newlcr) != OK)
	{
		printf("Error writing LINE_CTRL_REG\n");
		return 1;
	}
	if(sys_inb(com_address + DIVISOR_LATCH_LSB, &lsb) != OK)
	{
		printf("Error reading DIVISOR_LATCH_LSB\n");
		return 1;
	}
	if(sys_inb(com_address + DIVISOR_LATCH_MSB, &msb) != OK)
	{
		printf("Error reading DIVISOR_LATCH_MSB\n");
		return 1;
	}
	if(sys_outb(com_address + LINE_CTRL_REG, lcr) != OK)
	{
		printf("Error writing LINE_CTRL_REG\n");
		return 1;
	}
	*dl = (msb << 8) + lsb;
	return 0;
}

int set_divisor_latch(unsigned long com_address,unsigned long dl)
{
	unsigned long lsb = dl & 0xFF;
	unsigned long msb = dl >> 8;
	unsigned long lcr = 0;
	if(sys_inb(com_address + LINE_CTRL_REG, &lcr) != OK)
	{
		printf("Error reading LINE_CTRL_REG\n");
		return 1;
	}
	unsigned long newlcr = lcr | DLAB;
	if(sys_outb(com_address + LINE_CTRL_REG, newlcr) != OK)
	{
		printf("Error writing LINE_CTRL_REG\n");
		return 1;
	}
	if(sys_outb(com_address + DIVISOR_LATCH_LSB, lsb) != OK)
	{
		printf("Error writing DIVISOR_LATCH_LSB\n");
		return 1;
	}
	if(sys_outb(com_address + DIVISOR_LATCH_MSB, msb) != OK)
	{
		printf("Error writing DIVISOR_LATCH_MSB\n");
		return 1;
	}
	if(sys_outb(com_address + LINE_CTRL_REG, lcr) != OK)
	{
		printf("Error writing LINE_CTRL_REG\n");
		return 1;
	}
	return 0;
}

int ser_conf(unsigned short base_addr)
{
	if(base_addr > 2 || base_addr < 1)
		return 1;
	unsigned long com_address;
	if(base_addr == 1)
		com_address = COM1;
	else com_address = COM2;
	unsigned long lcr=0;
	if(sys_inb(com_address + LINE_CTRL_REG, &lcr) != OK)
	{
		printf("Error reading LINE_CTRL_REG\n");
		return 1;
	}
	unsigned long dl = 0;
	if(get_divisor_latch(com_address, &dl))
	{
		printf("Error reading DIVISOR_LATCH\n");
		return 1;
	}
	unsigned long intenable=0;
	if(sys_inb(com_address + INT_ENABLE_REG, &intenable) != OK)
	{
		printf("Error reading INT_ENABLE_REG\n");
		return 1;
	}
	printf("LCR: 0x%02x", lcr);
	switch(lcr & WORD_LENGTH_SELECT)
	{
	case WORD_LEN_5:
		printf("\t%d bits per character", 5);
		break;
	case WORD_LEN_6:
		printf("\t%d bits per character", 6);
		break;
	case WORD_LEN_7:
		printf("\t%d bits per character", 7);
		break;
	case WORD_LEN_8:
		printf("\t%d bits per character", 8);
		break;
	}
	switch(lcr & NO_STOP_BITS)
	{
	case STOP_BITS_1:
		printf("\t%d stop bits", 1);
		break;
	case STOP_BITS_2:
		printf("\t%d stop bits", 2);
		break;
	}
	switch(lcr & UART_PARITY)
	{
	case PARITY_ODD:
		printf("\t ODD parity\n");
		break;
	case PARITY_EVEN:
		printf("\t EVEN parity\n");
		break;
	case PARITY_1:
		printf("\t 1 parity\n");
		break;
	case PARITY_0:
		printf("\t 0 parity\n");
		break;
	default:
		printf("\tnone\n");
		break;
	}
	printf("DLM: 0x%02x ", dl&0xFFFF0000);
	printf("DLL: 0x%02x ", (dl&0xFFFF));
	printf("%d bps\n", UART_FREQ / dl);
	printf("IER: 0x%02x\t", intenable);
	if(intenable & ENABLE_RECEIVED_DATA_INT)
		printf("Rx ENABLED\t");
	else printf("Rx DISABLED\t");
	if(intenable & ENABLE_TRANSMITTER_EMPTY_INT)
		printf("Tx ENABLED\n");
	else printf("Tx DISABLED\n");
	return 0;
}

int ser_set(unsigned short base_addr, unsigned long bits, unsigned long stop,
        long parity, unsigned long rate)
{
	if(base_addr != 1 && base_addr != 2)
	{
		printf("Incorrect com: d%\n", base_addr);
		return 1;
	}
	if(bits < 5 || bits > 8)
	{
		printf("Incorrect number of bits: %d\n", bits);
		return 1;
	}
	if(rate != STD_RATE_VALUE_1 && rate != STD_RATE_VALUE_2 && rate != STD_RATE_VALUE_3 && rate !=STD_RATE_VALUE_4 && rate !=STD_RATE_VALUE_5 && rate != STD_RATE_VALUE_6 && rate != STD_RATE_VALUE_7  && rate!= STD_RATE_VALUE_8 )
	{
		printf("Incorrect rate: %d\n", rate);
		return 1;
	}
	if(stop != 1 && stop != 2)
	{
		printf("Incorrect number of stop bits: %d\n", parity);
		return 1;
	}
	if(parity < 0 || parity > 7)
	{
		printf ("Incorrect parity\n");
		return 1;
	}
	unsigned long com_address;
	if(base_addr == 1)
		com_address = COM1;
	else com_address = COM2;
	unsigned long writable_word_length = bits - 5;
	unsigned long writable_stop_bits = ((stop-1) << 2);
	unsigned long writable_parity = (parity << 3);
	unsigned long out = writable_parity|writable_stop_bits|writable_word_length;
	unsigned long lcr = 0;
	if(sys_inb(com_address + LINE_CTRL_REG, &lcr) != OK)
	{
		printf("Error reading LINE_CTRL_REG\n");
		return 1;
	}
	out |= (lcr & (DLAB|SET_BREAK_ENABLE));
	if(sys_outb(com_address + LINE_CTRL_REG, out) != OK)
	{
		printf("Error writing LINE_CTRL_REG\n");
		return 1;
	}

	unsigned long divisor = UART_FREQ / rate;
	if(set_divisor_latch(com_address, divisor))
	{
		printf("Error setting divisor_latch\n");
		return 1;
	}
	return 0;
}

int ser_set_receive_int_bit(unsigned long ser_port)
{
	unsigned long ier=0;
	if(sys_inb(ser_port + INT_ENABLE_REG, &ier) != OK)
	{
		printf("ser_set_receive_int_bit(): Error reading INT_ENABLE_REG\n");
		return 1;
	}
	ier |= (ENABLE_RECEIVED_DATA_INT|ENABLE_RECEIVER_LINE_STATUS_INT);
	if(sys_outb(ser_port + INT_ENABLE_REG, ier) != OK)
	{
		printf("ser_set_receive_int_bit(): Error writing to INT_ENABLE_REG\n");
		return 1;
	}
	return 0;
}

int ser_clear_receive_int_bit(unsigned long ser_port)
{
	unsigned long ier=0;
	if(sys_inb(ser_port + INT_ENABLE_REG, &ier) != OK)
	{
		printf("ser_clear_receive_int_bit(): Error reading INT_ENABLE_REG\n");
		return 1;
	}
	if(ier & ENABLE_RECEIVED_DATA_INT)
	ier ^= ENABLE_RECEIVED_DATA_INT;
	if(ier & ENABLE_RECEIVER_LINE_STATUS_INT)
		ier ^= ENABLE_RECEIVER_LINE_STATUS_INT;
	if(sys_outb(ser_port + INT_ENABLE_REG, ier) != OK)
	{
		printf("ser_clear_receive_int_bit(): Error writing to INT_ENABLE_REG\n");
		return 1;
	}
	return 0;
}

int ser_set_transmit_int_bit(unsigned long ser_port)
{
	unsigned long ier=0;
	if(sys_inb(ser_port + INT_ENABLE_REG, &ier) != OK)
	{
		printf("ser_set_receive_int_bit(): Error reading INT_ENABLE_REG\n");
		return 1;
	}
	ier |= (ENABLE_TRANSMITTER_EMPTY_INT|ENABLE_RECEIVER_LINE_STATUS_INT);
	if(sys_outb(ser_port + INT_ENABLE_REG, ier) != OK)
	{
		printf("ser_set_receive_int_bit(): Error writing to INT_ENABLE_REG\n");
		return 1;
	}
	return 0;
}

int ser_clear_transmit_int_bit(unsigned long ser_port)
{
	unsigned long ier=0;
	if(sys_inb(ser_port + INT_ENABLE_REG, &ier) != OK)
	{
		printf("ser_clear_receive_int_bit(): Error reading INT_ENABLE_REG\n");
		return 1;
	}
	if(ier & ENABLE_TRANSMITTER_EMPTY_INT)
	ier ^= ENABLE_TRANSMITTER_EMPTY_INT;
	if(ier & ENABLE_RECEIVER_LINE_STATUS_INT)
		ier ^= ENABLE_RECEIVER_LINE_STATUS_INT;
	if(sys_outb(ser_port + INT_ENABLE_REG, ier) != OK)
	{
		printf("ser_clear_receive_int_bit(): Error writing to INT_ENABLE_REG\n");
		return 1;
	}
	return 0;
}

int ser_set_line_int_bit(unsigned long ser_port)
{
	unsigned long ier=0;
	if(sys_inb(ser_port + INT_ENABLE_REG, &ier) != OK)
	{
		printf("ser_set_receive_int_bit(): Error reading INT_ENABLE_REG\n");
		return 1;
	}
	ier |= ENABLE_RECEIVER_LINE_STATUS_INT;
	if(sys_outb(ser_port + INT_ENABLE_REG, ier) != OK)
	{
		printf("ser_set_receive_int_bit(): Error writing to INT_ENABLE_REG\n");
		return 1;
	}
	return 0;
}

int ser_clear_line_int_bit(unsigned long ser_port)
{
	unsigned long ier=0;
	if(sys_inb(ser_port + INT_ENABLE_REG, &ier) != OK)
	{
		printf("ser_clear_receive_int_bit(): Error reading INT_ENABLE_REG\n");
		return 1;
	}
	if(ier & ENABLE_RECEIVER_LINE_STATUS_INT)
	ier ^= ENABLE_RECEIVER_LINE_STATUS_INT;
	if(sys_outb(ser_port + INT_ENABLE_REG, ier) != OK)
	{
		printf("ser_clear_receive_int_bit(): Error writing to INT_ENABLE_REG\n");
		return 1;
	}
	return 0;
}

int ser_subscribe_com1_int(void)
{
	int returnvalue = BIT(ser_com1_hook_id);
	if(sys_irqsetpolicy(ORIGINAL_SER_COM1_HOOK_ID,IRQ_EXCLUSIVE | IRQ_REENABLE, &ser_com1_hook_id)!= OK){
		printf("ser_subscribe_com1_int(): sys_irqsetpolicy() failed\n");
		return -1;
	}

	if(sys_irqenable(&ser_com1_hook_id)!=OK){
		printf("ser_subscribe_com1_int(): sys_irqenable() failed\n");
		return -1;
	}
	return returnvalue;//return value with bit number (original hook_id) set to 1
}

int ser_unsubscribe_com1_int(void) {
	if(sys_irqdisable(&ser_com1_hook_id) != OK){
		printf("ser_subscribe_com1_int(): sys_irqdisable() failed\n");
		return 1;
	}
	if(sys_irqrmpolicy(&ser_com1_hook_id) != OK){
		printf("ser_subscribe_com1_int(): sys_irqrmpolicy() failed\n");
		return 1;
	}
	return 0;
}

int ser_subscribe_com2_int(void)
{
	int returnvalue = BIT(ser_com2_hook_id);
	if(sys_irqsetpolicy(ORIGINAL_SER_COM2_HOOK_ID,IRQ_EXCLUSIVE | IRQ_REENABLE, &ser_com2_hook_id)!= OK){
		printf("ser_subscribe_com2_int(): sys_irqsetpolicy() failed\n");
		return -1;
	}
	if(sys_irqenable(&ser_com2_hook_id)!=OK){
		printf("ser_subscribe_com2_int(): sys_irqenable() failed\n");
		return -1;
	}
	return returnvalue;//return value with bit number (original hook_id) set to 1
}

int ser_unsubscribe_com2_int(void) {
	if(sys_irqdisable(&ser_com2_hook_id) != OK){
		printf("ser_subscribe_com2_int(): sys_irqdisable() failed\n");
		return 1;
	}
	if(sys_irqrmpolicy(&ser_com2_hook_id) != OK){
		printf("ser_subscribe_com2_int(): sys_irqrmpolicy() failed\n");
		return 1;
	}
	return 0;
}

int initialize_fifos(void)
{
	receive_fifo = new_char_queue_t(receive_fifo);
	transmit_fifo = new_char_queue_t(transmit_fifo);
	unsigned long fcr = 0;
	if(sys_inb(COM1+FIFO_CTRL_REG, &fcr) != OK)
	{
		printf("Error reading FIFO_CTRL_REG\n");
		return 1;
	}
	fcr |= FIFO_INIT;
	if(sys_outb(COM1+FIFO_CTRL_REG, fcr) != OK)
	{
		printf("Error writing to FIFO_CTRL_REG\n");
		return 1;
	}
	if(sys_inb(COM2+FIFO_CTRL_REG, &fcr) != OK)
	{
		printf("Error reading FIFO_CTRL_REG\n");
		return 1;
	}
	fcr |= FIFO_INIT;
	if(sys_outb(COM2+FIFO_CTRL_REG, fcr) != OK)
	{
		printf("Error writing to FIFO_CTRL_REG\n");
		return 1;
	}
	return 0;
}

int generic_ser_subscribe_int(unsigned long ser_port, unsigned char tx)
{
	int returnvalue = 0;
	if(tx == 0)
	{
		receiver_port =ser_port;
		if(ser_set_receive_int_bit(ser_port))
		{
			printf("generic_ser_subscribe_int(): ser_set_receive_int_bit() failed\n");
			return -1;
		}
	}
	else{
		transmit_port = ser_port;
		if(ser_set_line_int_bit(ser_port))
		{
			printf("generic_ser_subscribe_int(): ser_set_line_int_bit() failed\n");
			return -1;
		}
		if(ser_set_transmit_int_bit(ser_port))
		{
			printf("generic_ser_subscribe_int(): ser_set_transmit_int_bit() failed\n");
			return -1;
		}
	}
	if(ser_port == COM1)
	{
		if((returnvalue = ser_subscribe_com1_int()) < 0)
		{
			printf("generic_ser_subscribe_int(): ser_subscribe_com1_int() failed\n");
			return -1;
		}
	}
	else
	{
		if ((returnvalue = ser_subscribe_com2_int()) < 0)
		{
			printf("generic_ser_subscribe_int(): ser_subscribe_com2_int() failed\n");
			return -1;
		}
	}
	if(initialize_fifos())
		return -1;
	return returnvalue;
}

int generic_ser_unsubscribe_int(unsigned long ser_port, unsigned char tx)
{
	if(tx == 0)
	{
		if(ser_clear_receive_int_bit(ser_port))
		{
			printf("generic_ser_unsubscribe_int(): ser_clear_receive_int_bit() failed\n");
			return 1;
		}
	}
	else
	{

		if(ser_clear_line_int_bit(ser_port))
		{
			printf("generic_ser_unsubscribe_int(): ser_clear_line_int_bit() failed\n");
			return -1;
		}
		if(ser_clear_transmit_int_bit(ser_port))
		{
			printf("generic_ser_unsubscribe_int(): ser_clear_transmit_int_bit() failed\n");
			return 1;
		}
	}
	if(ser_port == COM1)
	{
		if(ser_unsubscribe_com1_int())
		{
			printf("generic_ser_unsubscribe_int(): ser_unsubscribe_com1_int() failed\n");
			return 1;
		}
	}
	else
	{
		if (ser_unsubscribe_com2_int())
		{
			printf("generic_ser_unsubscribe_int(): ser_unsubscribe_com2_int() failed\n");
			return 1;
		}
	}
	return 0;
}
int generic_ser_subscribe_int_tx_rx(unsigned long ser_port)
{
	transmit_port = ser_port;
	receiver_port = ser_port;
	int returnvalue = 0;
	if(ser_set_receive_int_bit(ser_port))
	{
		printf("generic_ser_subscribe_int(): ser_set_receive_int_bit() failed\n");
		return -1;
	}
	if(ser_set_line_int_bit(ser_port))
	{
		printf("generic_ser_subscribe_int(): ser_set_line_int_bit() failed\n");
		return -1;
	}
	if(ser_set_transmit_int_bit(ser_port))
	{
		printf("generic_ser_subscribe_int(): ser_set_transmit_int_bit() failed\n");
		return -1;
	}
	if(ser_port == COM1)
	{
		if((returnvalue = ser_subscribe_com1_int()) < 0)
		{
			printf("generic_ser_subscribe_int(): ser_subscribe_com1_int() failed\n");
			return -1;
		}
	}
	else
	{
		if ((returnvalue = ser_subscribe_com2_int()) < 0)
		{
			printf("generic_ser_subscribe_int(): ser_subscribe_com2_int() failed\n");
			return -1;
		}
	}
	if(initialize_fifos())
		return -1;
	return returnvalue;
}

int generic_ser_unsubscribe_int_tx_rx(unsigned long ser_port)
{
	return generic_ser_unsubscribe_int(ser_port, 1);
	if(ser_clear_receive_int_bit(ser_port))
	{
		printf("generic_ser_unsubscribe_int(): ser_clear_receive_int_bit() failed\n");
		return 1;
	}

	if(ser_clear_line_int_bit(ser_port))
	{
		printf("generic_ser_unsubscribe_int(): ser_clear_line_int_bit() failed\n");
		return -1;
	}
	if(ser_clear_transmit_int_bit(ser_port))
	{
		printf("generic_ser_unsubscribe_int(): ser_clear_transmit_int_bit() failed\n");
		return 1;
	}
	if(ser_port == COM1)
	{
		if(ser_unsubscribe_com1_int())
		{
			printf("generic_ser_unsubscribe_int(): ser_unsubscribe_com1_int() failed\n");
			return 1;
		}
	}
	else
	{
		if (ser_unsubscribe_com2_int())
		{
			printf("generic_ser_unsubscribe_int(): ser_unsubscribe_com2_int() failed\n");
			return 1;
		}
	}
	return 0;
}

int receiver_is_ready(unsigned long com_address)
{
	unsigned long lsr=0;
	if(sys_inb(com_address + LINE_STATUS_REG, &lsr) != OK)
	{
		printf("Error reading LINE_STATUS_REG\n");
		return 1;
	}
	return (lsr & RECEIVER_READY);
}
int transmitter_is_ready(unsigned long com_address)
{
	unsigned long lsr=0;
	if(sys_inb(com_address + LINE_STATUS_REG, &lsr) != OK)
	{
		printf("Error reading LINE_STATUS_REG\n");
		return 1;
	}
	return (lsr & TRANSMITTER_HOLDING_REG_EMPTY);
}
int ser_ih(unsigned long ser_port) {
	unsigned long iir;
	if(sys_inb(ser_port + INT_ID_REG, &iir))
	{
		printf("ser_ih() failed\n");
		return 1;
	}
	if((iir & INT_STATUS)  == 0) {
		switch(iir & INT_ORIGIN) {
		case RECEIVED_DATA_AVAILABLE_INT:
			while(receiver_is_ready(ser_port))
			{
				if(sys_inb(ser_port+RECEIVER_BUF_REG, &received_char) != OK)
				{
					printf("Error reading RECEIVER_BUF_REG\n");
					return 1;
				}
				push(receive_fifo, received_char);
			}
			break;
		case TRANSMITTER_EMPTY_INT:
			while(!is_empty_q(transmit_fifo) && transmitter_is_ready(ser_port)){
				if(sys_outb(ser_port+TRANSMITTER_HOLDING_REG, front(transmit_fifo)) != OK)
				{
					printf("Error writing TRANSMITTER_HOLDING_REG\n");
					return 1;
				}
				pop(transmit_fifo);
			}
			break;
		case CHAR_TIMEOUT_INT:
			return 1;
			break;
		case LINE_STATUS_INT:
			return 1;
			break;
		}
	}
	return 0;
}
int transmit_b(unsigned char byte)
{
	push(transmit_fifo, byte);
	while(!is_empty_q(transmit_fifo) && transmitter_is_ready(transmit_port)){
		if(sys_outb(transmit_port+TRANSMITTER_HOLDING_REG, front(transmit_fifo)) != OK)
		{
			printf("Error writing TRANSMITTER_HOLDING_REG\n");
			return 1;
		}
		pop(transmit_fifo);
	}
	return 0;
}
int transmit_m(void * data, size_t size)
{
	push_m(transmit_fifo, data, size);
	while(!is_empty_q(transmit_fifo) && transmitter_is_ready(transmit_port)){
		if(sys_outb(transmit_port+TRANSMITTER_HOLDING_REG, front(transmit_fifo)) != OK)
		{
			printf("Error writing TRANSMITTER_HOLDING_REG\n");
			return 1;
		}
		pop(transmit_fifo);
	}
	return 0;
}
int receive_b(unsigned char* received)
{
	while(receiver_is_ready(receiver_port))
	{
		if(sys_inb(receiver_port+RECEIVER_BUF_REG, &received_char) != OK)
		{
			printf("Error reading RECEIVER_BUF_REG\n");
			return 1;
		}
		push(receive_fifo, received_char);
	}
	if(is_empty_q(receive_fifo))
		return 1;
	*received = front(receive_fifo);
	pop(receive_fifo);

	return 0;
}
int receive_m(void * data, size_t size)
{
	while(receiver_is_ready(receiver_port))
	{
		if(sys_inb(receiver_port+RECEIVER_BUF_REG, &received_char) != OK)
		{
			printf("Error reading RECEIVER_BUF_REG\n");
			return 1;
		}
		push(receive_fifo, received_char);
	}
	if(size_q(receive_fifo) < size)
		return 1;
	front_m(receive_fifo, data, size);
	pop_m(receive_fifo, size);
	return 0;
}

int receive_protocol(void* data, signal_type_t* signal_type, size_t* size)
{
	while(receiver_is_ready(receiver_port))
	{
		if(sys_inb(receiver_port+RECEIVER_BUF_REG, &received_char) != OK)
		{
			printf("Error reading RECEIVER_BUF_REG\n");
			return 1;
		}
		push(receive_fifo, received_char);
	}
	if(size_q(receive_fifo) == 0)
		return 1;
	*signal_type = front(receive_fifo);
	char* signal_type_and_size;
	switch(*signal_type)
	{
	case TYPE_INT:
		if(size_q(receive_fifo) < 1 + sizeof(int))
			return 1;
		*size = sizeof(int);
		pop(receive_fifo);
		receive_m(data,*size);
		break;
	case TYPE_SHORT:
		if(size_q(receive_fifo) < 1 + sizeof(short))
			return 1;
		*size = sizeof(short);
		pop(receive_fifo);
		receive_m(data,*size);
		break;
	case TYPE_CHAR:
		if(size_q(receive_fifo) < 1 + sizeof(short))
			return 1;
		*size = sizeof(char);
		pop(receive_fifo);
		receive_m(data,*size);
		break;
	case TYPE_STRING:
		signal_type_and_size = (char*) malloc(sizeof(unsigned char)+ sizeof(size_t));
		front_m(receive_fifo, signal_type_and_size, sizeof(unsigned char)+ sizeof(size_t));
		signal_type_and_size++;
		*size = *(size_t*) signal_type_and_size;
		if(size_q(receive_fifo) < sizeof(unsigned char) + sizeof(size_t) + *size)
			return 1;
		pop_m(receive_fifo, sizeof(unsigned char)+ sizeof(size_t));
		receive_m(data,*size);
		free(--signal_type_and_size);
		break;
	case TYPE_MESSAGE:
		if(size_q(receive_fifo) < 1 + sizeof(unsigned char))
			return 1;
		pop(receive_fifo);
		*(unsigned char*) data = front(receive_fifo);
		pop(receive_fifo);
		*size = 1;
		break;
	case TYPE_FLOAT:
		if(size_q(receive_fifo) < 1 + sizeof(float))
			return 1;
		*size = sizeof(float);
		pop(receive_fifo);
		receive_m(data,*size);
		break;
	case TYPE_DOUBLE:
		if(size_q(receive_fifo) < 1 + sizeof(double))
			return 1;
		*size = sizeof(double);
		pop(receive_fifo);
		receive_m(data,*size);
		break;
	default:
		pop(receive_fifo); //unexpected signal
		return 1;
		break;
	}
	return 0;
}
int transmit_protocol(void* data, signal_type_t signal_type, size_t size)
{
	transmit_b((unsigned char) signal_type);
	switch(signal_type)
	{
	case TYPE_INT:
		transmit_m(data, sizeof(int));
		break;
	case TYPE_SHORT:
		transmit_m(data, sizeof(short));
		break;
	case TYPE_CHAR:
		transmit_b(*(unsigned char*)data);
		break;
	case TYPE_STRING:
		transmit_m(&size, sizeof(size_t));
		transmit_m(data, size);
		break;
	case TYPE_MESSAGE:
		transmit_b(*(unsigned char*)data);
		break;
	case TYPE_FLOAT:
		transmit_m(data, sizeof(float));
		break;
	case TYPE_DOUBLE:
		transmit_m(data, sizeof(double));
		break;
	}
	while(!is_empty_q(transmit_fifo) && transmitter_is_ready(transmit_port)){
		if(sys_outb(transmit_port+TRANSMITTER_HOLDING_REG, front(transmit_fifo)) != OK)
		{
			printf("Error writing TRANSMITTER_HOLDING_REG\n");
			return 1;
		}
		pop(transmit_fifo);
	}
	return 0;
}

int transmit_message(message_type_t message)
{
	return transmit_protocol(&message, TYPE_MESSAGE, sizeof(unsigned short));
}
