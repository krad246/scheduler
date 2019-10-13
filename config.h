/*
 * config.h
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <msp430.h>

#include <cstdint>
#include <cstddef>

#define DEBUG_MODE
#define INT_QUEUE_SIZE 32

/**
 * Declare your functions here
 */

#define decl_function(name) std::int16_t name(void)

decl_function(foo);
decl_function(bar);
decl_function(printer1);
decl_function(printer2);
decl_function(printer3);
decl_function(printer4);
decl_function(fib);

/**
 * Count number of declared tasks
 */

#define num_tasks_declared 4

/**
 * Struct declaration for task customization array
 */

using runnable = std::int16_t (*)(void);

struct task_config {
	runnable func;
	std::size_t stack_size;
	std::uint8_t priority;
};

extern struct task_config task_cfgs[];

/**
 * List of available scheduling algorithms
 */

enum class scheduling_algorithms {
	round_robin,
	lottery
};

#endif /* CONFIG_H_ */
