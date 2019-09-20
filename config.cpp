/*
 * config.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#include <config.h>
#include <scheduler.h>

/**
 * Select the scheduler and the type of scheduling algorithm to use
 */

scheduler<scheduling_algorithms::round_robin> os;

/**
 * Populate this list with the configurations of each task
 */

struct task_config task_cfgs[num_tasks_declared] = {
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
