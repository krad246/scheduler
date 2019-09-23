/*
 * print.c
 *
 *  Created on: Feb 8, 2019
 *      Author: krad2
 */

#include <print.h>
#include <scheduler.h>

extern scheduler<scheduling_algorithms::round_robin> os;

ring_buffer<char> rx_fifo(32);

/**
 * Interrupt routine for receiving a character over UART
 **/

#pragma vector = USCI_A1_VECTOR
__attribute__((interrupt)) void USCI_A1_ISR(void) {

	auto old_sp = os.enter_kstack();

	switch (__even_in_range(UCA1IV, 4)) {
		case 0: { // Vector 0 - no interrupt
			break;
		}

		case 2: { // Vector 2 - RXIFG
			rx_fifo.put(UCA1RXBUF);
			if (rx_fifo.full()) {
				os.schedule_interrupt(USCI_A1_ISR);
			}

			UCA1IFG &= ~UCRXIFG;
			break;
		}

		case 4: { // Vector 4 - TXIFG
			break;
		}

		default: {
			break;
		}
	}

	os.leave_kstack(old_sp);
}

std::int16_t uart_task(void) {
	while (1) {
		os.refresh();
		uart_printf("%s", "got characters\n\r");
		while (!rx_fifo.empty()) {
			uart_putc(rx_fifo.get());
		}

		os.block();
	}
}

/**
 * Initializes the UART for 115200 baud with a RX interrupt
 **/

void uart_init(void) {
	  P4SEL |= BIT4 + BIT5;                     // P3.3,4 = USCI_A0 TXD/RXD
	  UCA1CTL1 |= UCSWRST;                      // Put state machine in reset
	  UCA1CTL1 |= UCSSEL_2;                     // SMCLK
	  UCA1BR0 = 9;                              // 1MHz 115200 (see User's Guide)
	  UCA1BR1 = 0;                              // 1MHz 115200
	  UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	  UCA1CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
	  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

	  os.attach_interrupt(USCI_A1_ISR, task(uart_task, 32));
}

/**
 * uart_puts() is used by printf() to display or send a string
 **/

void uart_puts(char *s) {
	for (char *p = s; *p != 0; p++) uart_send_byte(*p);
}

/**
 * uart_puts() is used by printf() to display or send a character
 **/

void uart_putc(unsigned b) {
	uart_send_byte(b);
}

/**
 * Sends a single byte out through UART
 **/

void uart_send_byte(unsigned char byte) {
	while (UCA1STAT & UCBUSY);
	UCA1TXBUF = byte;
}

static const unsigned long dv[] = {
//  4294967296      // 32 bit unsigned max
		1000000000,// +0
		100000000, // +1
		10000000, // +2
		1000000, // +3
		100000, // +4
//       65535      // 16 bit unsigned max
		10000, // +5
		1000, // +6
		100, // +7
		10, // +8
		1, // +9
		};

static void xtoa(unsigned long x, const unsigned long *dp) {
	char c;
	unsigned long d;
	if (x) {
		while (x < *dp)
			++dp;
		do {
			d = *dp++;
			c = '0';
			while (x >= d)
				++c, x -= d;
			uart_putc(c);
		} while (!(d & 1));
	} else
		uart_putc('0');
}

static void puth(unsigned n) {
	static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
			'9', 'A', 'B', 'C', 'D', 'E', 'F' };
	uart_putc(hex[n & 15]);
}

void uart_printf(char *format, ...)
{
	char c;
	int i;
	long n;

	va_list a;
	va_start(a, format);
	while (c = *format++) {
		if (c == '%') {
			switch (c = *format++) {
				case 's': // String
					uart_puts(va_arg(a, char*));
					break;
				case 'c':// Char
					uart_putc(va_arg(a, char));
				break;
				case 'i':// 16 bit Integer
				case 'u':// 16 bit Unsigned
					i = va_arg(a, int);
					if (c == 'i' && i < 0) i = -i, uart_putc('-');
					xtoa((unsigned)i, dv + 5);
				break;
				case 'l':// 32 bit Long
				case 'n':// 32 bit uNsigned loNg
					n = va_arg(a, long);
					if (c == 'l' && n < 0) n = -n, uart_putc('-');
					xtoa((unsigned long)n, dv);
				break;
				case 'x':// 16 bit heXadecimal
					i = va_arg(a, int);
					puth(i >> 12);
					puth(i >> 8);
					puth(i >> 4);
					puth(i);
				break;
				case 0: return;
				default: goto bad_fmt;
			}
		} else
			bad_fmt: uart_putc(c);
	}
	va_end(a);
}
