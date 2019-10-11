/*
 * print.c
 *
 *  Created on: Feb 8, 2019
 *      Author: krad2
 */

#include <print.h>
#include <scheduler.h>

extern scheduler<scheduling_algorithms::round_robin> os;

ring_buffer<char> rx_fifo(16);
ring_buffer<char> tx_fifo(16);

/**
 * Interrupt routine for receiving a character over UART
 **/

#pragma vector = USCI_A1_VECTOR
__attribute__((interrupt)) void USCI_A1_ISR(void) {
	switch (__even_in_range(UCA1IV, 4)) {
		case 0: { // Vector 0 - no interrupt
			break;
		}

		case 2: { // Vector 2 - RXIFG
			char recv = UCA1RXBUF;

			P4OUT |= BIT7;
			rx_fifo.put(recv);
			P4OUT &= ~BIT7;

			P1OUT |= BIT0;
			uart_send_byte(recv);
			P1OUT &= ~BIT0;

			if (rx_fifo.full()) os.schedule_interrupt((isr) uart_rx_task); // when max(character requests) received, then schedule the rx task
			break;
		}

		case 4: { // Vector 4 - TXIFG

			if (!tx_fifo.empty()) {
				P1OUT |= BIT0;
				uart_send_byte(tx_fifo.get());
				P1OUT &= ~BIT0;
			} else {
				UCA1IE &= ~UCTXIE;
				UCA1IFG &= ~UCTXIFG;
			}
			break;
		}

		default: {
			break;
		}
	}
}

std::int16_t uart_tx_task(void) {
	while (1) {
		_disable_interrupt();
		if (!tx_fifo.empty()) {
			while (UCA1STAT & UCBUSY);
			UCA1IE |= UCTXIE;                         // Enable USCI_A0 TX interrupt
			uart_send_byte(tx_fifo.get());
		} else {
			os.suspend();
		}
		_enable_interrupt();
	}
}

std::int16_t uart_rx_task(void) {
	while (1) {
		_disable_interrupt();
		// copy rx characters to all processes waiting
		_enable_interrupt();

		_disable_interrupt();
		rx_fifo.reset();
		_enable_interrupt();

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

	P4DIR |= BIT7;
	P4OUT &= ~BIT7;
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	os.add_task(std::move(task(uart_tx_task, 64)));
	os.attach_interrupt((isr) uart_rx_task, task(uart_rx_task, 64));
}

/**
 * uart_puts() is used by printf() to display or send a string
 **/

void uart_puts(char *s) {
	for (char *p = s; *p != 0; p++) uart_putc(*p);
}

/**
 * uart_puts() is used by printf() to display or send a character
 **/

void uart_putc(unsigned b) {
	_disable_interrupt();
	while (tx_fifo.full()) {
		os.suspend();
	}
	_enable_interrupt();

	_disable_interrupt();
	tx_fifo.put(b);
	_enable_interrupt();
}

/**
 * Sends a single byte out through UART
 **/

void uart_send_byte(unsigned char byte) {
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
			if (d == 0) break;
			c = '0';
			while (x >= d) {
				++c;
				x -= d;
			}

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
