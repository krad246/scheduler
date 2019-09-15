#include <msp430.h> 
#include <task.h>
#include <scheduler.h>
#include <print.h>

std::int16_t foo(void);
std::int16_t bar(void);
std::int16_t printer(void);

scheduler<scheduling_algorithms::round_robin> os;

std::int16_t foo(void) {
	volatile std::uint32_t q = 31;

	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	while (1) {
		P1OUT ^= BIT0;
		q++;
		os.sleep(64);
	}
}

std::int16_t bar(void) {
	volatile std::uint32_t q = 31;

	P4DIR |= BIT7;
	P4OUT &= ~BIT7;

	while (1) {
		P4OUT ^= BIT7;
		q++;
		os.sleep(1);
	}
}

std::int16_t printer(void) {
	while (1) {
		puts("Hello, world!");
		os.sleep(4);
	}
}

// task that returns must have return address planted at base of user stack pointing to cleanup function

// enter critical section
// cleanup function switches to scheduler sp
// pop current task from list of tasks
// schedule new task
// end critical section
// load new task

// interrupt policy must pass messages to tasks that request
// calls to drivers push task identifier onto queue matched to desired interrupt
// calling tasks block on driver calls, interrupts reawaken tasks & send message to task

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void bob(void) {
//	x.pause();
	os.enter_kernel_mode();
	os.leave_kernel_mode();

	// switch sp to scheduler stack pointer

	// carry out tasks

	// calculate next task
	// back up sp current location before calla

	// jump to new task
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	WDTCTL = WDT_ADLY_16;
	SFRIE1 |= WDTIE;
	initUART();
	task x = task(foo, 32);
	task y = task(bar, 32);
	task z = task(printer, 100);

	os.start({x, y});
	return 0;
}



// initialize all tasks
// pick a starter task
// set stack pointer to that task's user stack
// unconditionally branch to that task

