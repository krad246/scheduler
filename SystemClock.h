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

class SystemClock {
public:
	static void StartSystemClock(std::size_t frequency = (std::size_t) 1000000);
	static void StartCrystalOscillator(std::size_t frequency = (std::size_t) 1000000);

private:
	friend class Scheduler;

	static std::size_t microsPerTick;

	static std::size_t micros;
	static std::size_t fractionalMillis;
	static std::size_t millis;
};

#endif /* SYSTEMCLOCK_H_ */
