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
 * Struct declaration for task customization array
 */

using runnable = std::int16_t (*)(void);

struct task_config {
	runnable func;
	std::size_t stack_size;
	std::uint8_t priority;
};

/**
 * Populate this list with the configurations of each task
 */

constexpr const struct task_config task_cfgs[] = {
		{
				.func = foo,
				.stack_size = 32,
				.priority = 1
		},
		{
				.func = bar,
				.stack_size = 32,
				.priority = 2
		},
		{
				.func = printer1,
				.stack_size = 32,
				.priority = 3,
		},
		{
				.func = printer2,
				.stack_size = 32,
				.priority = 4,
		},
		{
				.func = printer3,
				.stack_size = 32,
				.priority = 5,
		},
		{
				.func = printer4,
				.stack_size = 32,
				.priority = 6,
		},
		{
				.func = fib,
				.stack_size = 32,
				.priority = 7,

		}
};

/**
 * List of available scheduling algorithms
 */

enum class scheduling_algorithms {
	round_robin,
	lottery
};

#endif /* CONFIG_H_ */
