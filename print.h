/*
 * print.h
 *
 *  Created on: Feb 8, 2019
 *      Author: krad2
 */

#ifndef PRINT_H_
#define PRINT_H_

#include <msp430.h>
#include <cstdarg>

#include <ring_buffer.h>

void uart_putc(unsigned);
void uart_puts(char *);
void uart_send_byte(unsigned char byte);
void uart_printf(char *format, ...);

void uart_init(void);

class uart {

private:
	ring_buffer<char> tx_fifo;
	ring_buffer<char> rx_fifo;
};

#endif /* PRINT_H_ */
