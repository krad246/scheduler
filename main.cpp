#include <msp430.h> 
#include <task.h>
#include <scheduler.h>
#include <print.h>

extern scheduler<scheduling_algorithms::round_robin> os;

void driver_init(void) {
	uart_init();
}

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void bob(void) {
	os.enter_kernel_mode();
	os.leave_kernel_mode();
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	driver_init();

	os.init();
	os.start();
	return 0;
}
