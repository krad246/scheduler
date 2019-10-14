/*
 * watchdog.cpp
 *
 *  Created on: Oct 14, 2019
 *      Author: krad2
 */

#include <watchdog.h>

// Configure the scheduler timer interrupt
void watchdog_init(void) {
	WDTCTL = WDT_ADLY_1_9;
	SFRIE1 |= WDTIE;
}

// Set preemption flag and wait for interrupts
void watchdog_request(void) {
	SFRIFG1 |= WDTIFG;
}

// Disable watchdog because it must be reloaded upon the end of a context switch
void watchdog_reload(void) {
	SFRIFG1 &= ~WDTIFG;
	WDTCTL = WDT_ADLY_1_9;
}

void wdt_reload(void) {
	watchdog_reload();
}
