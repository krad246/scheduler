/*
 * SystemClock.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef SYSTEMCLOCK_H_
#define SYSTEMCLOCK_H_

#include <msp430.h>
#include <cstddef>
#include <cstdint>

/**
 * SystemClock class:
 * Class representing the system tick and other non-peripheral timing modules on the device.
 */

class SystemClock {
public:

	/**
	 * Function to configure the clock module & scheduler tick. Also determines timing characteristics
	 * for the system millisecond and microsecond timers.
	 */

	static void StartSystemClock(std::size_t frequency = (std::size_t) 1000000);

	/**
	 * Function to start the crystal oscillator peripheral device on the board.
	 */

	static void StartCrystalOscillator(std::size_t frequency = (std::size_t) 1000000);

	/**
	 * Updates the internal timing state of the system in milliseconds and microseconds.
	 */

	static void UpdateSystemTime(void);

private:
	friend class Scheduler;

	/**
	 * Timing variables for the system.
	 */

	static std::size_t microsPerTick;
	static std::size_t micros;
	static std::size_t fractionalMillis;
	static std::size_t millis;
};

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

#endif /* SYSTEMCLOCK_H_ */
