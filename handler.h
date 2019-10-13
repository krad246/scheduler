/*
 * handler.h
 *
 *  Created on: Oct 12, 2019
 *      Author: krad2
 */

#ifndef HANDLER_H_
#define HANDLER_H_

#include <task.h>

class handler : public task {
public:
	handler(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t latency = 0);

	bool ready(void);

private:
	std::uint8_t latency;
};

#endif /* HANDLER_H_ */
