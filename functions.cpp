/*
 * functions.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#include <scheduler.h>

/**
 * Must reference the selected scheduler here
 */

extern scheduler<scheduling_algorithms::round_robin> os;

/**
 * Include additional headers here
 */

#include <print.h>

/**
 * Fill out your function bodies here
 */

std::int16_t foo(void) {
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	while (1) {
		P1OUT ^= BIT0;
		uart_printf("%i\n\r", os.get_thread_state().ticks);
		os.sleep(64);
	}
}

std::int16_t bar(void) {
	P4DIR |= BIT7;
	P4OUT &= ~BIT7;

	while (1) {
		P4OUT ^= BIT7;
		uart_printf("%i\n\r", os.get_thread_state().ticks);
		os.sleep(64);
	}
}

std::int16_t printer1(void) {
	while (1) {
		uart_printf("%i\n\r", os.get_thread_state().ticks);
		os.sleep(64);
	}
}

std::int16_t printer2(void) {
	while (1) {
		uart_printf("%i\n\r", os.get_thread_state().ticks);
		os.sleep(64);
	}
}

std::int16_t printer3(void) {
	while (1) {
		uart_printf("%i\n\r", os.get_thread_state().ticks);
		os.sleep(64);
	}
}

std::int16_t printer4(void) {
	while (1) {
		uart_printf("%i\n\r", os.get_thread_state().ticks);
		os.sleep(64);
	}
}

std::int16_t fib(void) {
	while (1) {
		uart_printf("%i\n\r", os.get_thread_state().ticks);
        os.sleep(64);
	}
}
