/*
 * handler.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: krad2
 */

#include <handler.h>

handler::handler(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t latency) {
	new (this) task::task(runnable, stack_size, latency, true);
}

bool handler::ready(void) {
	if (this->latency == 0) {
		if (!this->complete()) {
			this->latency = this->get_priority();
			return true;
		} else {
			return false;
		}
	} else {
		this->latency--;
	}
	return false;
}
