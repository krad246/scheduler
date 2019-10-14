#include <msp430.h> 
#include <task.h>
#include <scheduler.h>
#include <print.h>

extern scheduler<scheduling_algorithms::lottery> os;

void clk_init(void) {

}

void driver_init(void) {
	clk_init();
	uart_init();
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	os.init();
	os.start();
	return 0;
}
