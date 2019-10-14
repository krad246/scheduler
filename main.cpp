#include <msp430.h> 
#include <task.h>
#include <scheduler.h>
#include <print.h>

extern scheduler<scheduling_algorithms::lottery> os;

// Configure the scheduler timer interrupt
void watchdog_init(void) {
//	WDTCTL = WDT_ADLY_16;
	WDTCTL = WDT_ADLY_1_9;
	SFRIE1 |= WDTIE;
}

// Set preemption flag and wait for interrupts
void watchdog_request(void) {
	SFRIFG1 |= WDTIFG;
}

// Disable watchdog because it must be reloaded upon the end of a context switch
void watchdog_suspend(void) {
	SFRIFG1 &= ~WDTIFG;
}

void driver_init(void) {
	uart_init();
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	os.init();
	os.start();
	return 0;
}
