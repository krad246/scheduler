/*
 * SystemClock.cpp
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#include <SystemClock.h>

/**
 * Default initializations for the system time.
 */

std::size_t SystemClock::microsPerTick = 0;
std::size_t SystemClock::micros = 0;
std::size_t SystemClock::fractionalMillis = 0;
std::size_t SystemClock::millis = 0;

/**
 * Start the system timer up with the clock module running at some specified frequency.
 */

void SystemClock::StartSystemClock(std::size_t frequency) {

	/**
	 * Only 1 MHz, 8 MHz, 12 MHz, and 16 MHz are supported.
	 * If any of those frequencies in Hz are passed in, then configure the clock module accordingly.
	 * Otherwise, halt / crash.
	 */

	switch (frequency) {
	case (std::size_t) 1000000:
		DCOCTL = CALDCO_1MHZ;
		BCSCTL1 = CALBC1_1MHZ;
		break;
	case (std::size_t) 8000000:
		DCOCTL = CALDCO_8MHZ;
		BCSCTL1 = CALBC1_8MHZ;
		break;
	case (std::size_t) 12000000:
		DCOCTL = CALDCO_12MHZ;
		BCSCTL1 = CALBC1_12MHZ;
		break;
	case (std::size_t) 16000000:
		DCOCTL = CALDCO_16MHZ;
		BCSCTL1 = CALBC1_16MHZ;
		break;

	/**
	 * In the event that no frequency matches.
	 */

	default:
		_low_power_mode_4();
	}

	/**
	 * Depending on the clock frequency of the system clock, the watchdog timer frequency changes.
	 * SystemClock::microsPerTick is the number of microseconds per overflow of the timer.
	 */

	SystemClock::microsPerTick = frequency == 1000000 ? 512 : 8192;

	/**
	 * Configure the watchdog timer in interval mode with the value above and start it.
	 */

	if (frequency == 1000000) WDTCTL = WDT_MDLY_0_5;
	else WDTCTL = WDT_MDLY_8;

	/**
	 * Enable interrupts for the scheduler tick.
	 */

	IE1 |= WDTIE;
}

/**
 * Configures and initializes an optional crystal oscillator on the board.
 */

void SystemClock::StartCrystalOscillator(std::size_t frequency) {

	/**
	 * Try to clear the fault flags for the oscillator for configuration.
	 * If the crystal oscillator responds, then the flag will be unset.
	 * Otherwise the operation will fail. Try 4 times every 0.5 seconds.
	 */

	std::uint16_t timeout = 4;
	do {
		/**
		 * Clear the flags and try to enable the oscillator.
		 */

		BCSCTL3 &= ~LFXT1OF;
		IFG1 &= ~OFIFG;

		/**
		 * Depending on the clock frequency of the system, wait for a normalized 0.5 sec.
		 */

		switch (frequency) {
		case (std::size_t) 1000000:
			_delay_cycles(500000);
			break;
		case (std::size_t) 8000000:
			_delay_cycles(4000000);
			break;
		case (std::size_t) 12000000:
			_delay_cycles(6000000);
			break;
		case (std::size_t) 16000000:
			_delay_cycles(8000000);
			break;
		}

		timeout--;
		if (!timeout) break;
	} while (IFG1 & OFIFG);

	/**
	 * If initializing the oscillator is unsuccessful, default-initialize the board.
	 */

	if (!timeout) {
		P2SEL &= ~(BIT6 | BIT7);
		BCSCTL3 |= LFXT1S_2;
	}
}

/**
 * Updates system time every interrupt tick.
 */

inline void SystemClock::UpdateSystemTime(void) {

	/**
	 * One tick of the system clock takes some number of microseconds, so that is the time passage.
	 */

	micros += microsPerTick;

	/**
	 * Compute the microsecond value modulo 1 millisecond. Put the 'rest' in the millisecond timer.
	 */

	fractionalMillis += microsPerTick;
	if (fractionalMillis > 1000) {
		fractionalMillis -= 1000;
		millis += 1;
	}
}
