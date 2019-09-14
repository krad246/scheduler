/*
 * print.c
 *
 *  Created on: Feb 8, 2019
 *      Author: krad2
 */

#include <print.h>
interrupt void USCI_A1_ISR(void);
extern scheduler<scheduling_algorithms::round_robin> os;

/**
 * Initializes the UART for 115200 baud with a RX interrupt
 **/
void initUART(void) {
	  P4SEL |= BIT4 + BIT5;                       // P3.3,4 = USCI_A0 TXD/RXD
	  UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
	  UCA1CTL1 |= UCSSEL_2;                     // SMCLK
	  UCA1BR0 = 9;                              // 1MHz 115200 (see User's Guide)
	  UCA1BR1 = 0;                              // 1MHz 115200
	  UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	  UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

/**
 * puts() is used by printf() to display or send a string.. This function
 *     determines where printf prints to. For this case it sends a string
 *     out over UART, another option could be to display the string on an
 *     LCD display.
 **/
void puts(char *s) {
//	os.enter_kstack();
//	// block until tx buffer ready
	for (;;) {
		if (UCA1STAT & UCBUSY) os.block();
		else {
			os.unblock();
			break;
		}
	}

	_disable_interrupt();

//	// send pointer to string data to message queue of ISR
//	// isr increments pointer when ready
	isr_mailbox_retrieval ret = os.fetch_mailbox(USCI_A1_ISR);
	std::unordered_map<isr, std::queue<message>>::iterator tx_requests_ptr = ret.mailbox_ptr;
	std::queue<message> &tx_request_list = tx_requests_ptr->second;

	message tx_request(&os.get_current_process(), nullptr, reinterpret_cast<void *>(s));
	tx_request_list.push(tx_request);

	// send 1st character only
//	send_byte(*s);

//	os.leave_kstack();
//	SFRIE1 &= ~WDTIE;

	_enable_interrupt();
	UCA1IE |= UCTXIE;

//	for (;;) {
//		UCA1IE |= UCTXIE;
//	}

//	SFRIE1 |= WDTIE;

//	for (;;) {UCA1IE |= UCTXIE;
//	_enable_interrupt();}
//	for (;;) {UCA1IE |= UCTXIE;}
//	_low_power_mode_0();
}
/**
 * puts() is used by printf() to display or send a character. This function
 *     determines where printf prints to. For this case it sends a character
 *     out over UART.
 **/
void putc(unsigned b) {
	send_byte(b);
}

/**
 * Sends a single byte out through UART
 **/
void send_byte(unsigned char byte)
{
	UCA1TXBUF = byte;
}

/**
 * Interrupt routine for receiving a character over UART
 **/
// Echo back RXed character, confirm TX buffer is ready first
#pragma vector = USCI_A1_VECTOR
interrupt void USCI_A1_ISR(void) {
//	_disable_interrupt();
	// switch to system stack pointer
//	os.enter_kstack();

//	SFRIE1 &= ~WDTIE;

	switch (__even_in_range(UCA1IV, 4)) {
		case 0: { // Vector 0 - no interrupt
			break;
		}

		case 2: { // Vector 2 - RXIFG
			break;
		}

		case 4: { // Vector 4 - TXIFG
			// fetch tx requests list from message queue
			isr_mailbox_retrieval ret = os.fetch_mailbox(USCI_A1_ISR);
//			if (ret.ret_code < 0) {
//				// halt();
////				break;
//			}

			// retrieve the pointer to the entry and the actual list
			std::unordered_map<isr, std::queue<message>>::iterator tx_requests_ptr = ret.mailbox_ptr;
			std::queue<message> &tx_request_list = tx_requests_ptr->second;

			// if empty then return immediately
			if (tx_request_list.empty()) break;

			// otherwise get the earliest tx request and the pointer to the current character to send
			message &req = tx_request_list.front();
			const char *tx_character_ptr = reinterpret_cast<const char *>(req.data_ptr);

			// if we hit a null terminator, then this request has been served
			if (*tx_character_ptr == 0) {
				tx_request_list.pop();
				SFRIE1 |= WDTIE;
				// send message to calling task that it is done
				UCA1IE &= ~UCTXIE;
				break;
			} else {
				// otherwise move pointer forwards and send character
				req.data_ptr = const_cast<void *>(reinterpret_cast<const void *>(tx_character_ptr + 1));

				// send the character
//				UCA1IE &= ~UCTXIE;
				send_byte(*tx_character_ptr);
			}

			break;
		}

		default: {
			break;
		}
	}
//	_enable_interrupt();
//	os.leave_kstack();
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
			putc(c);
		} while (!(d & 1));
	} else
		putc('0');
}

static void puth(unsigned n) {
	static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
			'9', 'A', 'B', 'C', 'D', 'E', 'F' };
	putc(hex[n & 15]);
}

void print(char *format, ...)
{
	char c;
	int i;
	long n;

	va_list a;
	va_start(a, format);
	while(c = *format++) {
		if(c == '%') {
			switch(c = *format++) {
				case 's': // String
					puts(va_arg(a, char*));
					break;
				case 'c':// Char
					putc(va_arg(a, char));
				break;
				case 'i':// 16 bit Integer
				case 'u':// 16 bit Unsigned
					i = va_arg(a, int);
					if(c == 'i' && i < 0) i = -i, putc('-');
					xtoa((unsigned)i, dv + 5);
				break;
				case 'l':// 32 bit Long
				case 'n':// 32 bit uNsigned loNg
					n = va_arg(a, long);
					if(c == 'l' && n < 0) n = -n, putc('-');
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
			bad_fmt: putc(c);
	}
	va_end(a);
}
