/*
 * task.h
 *
 *  Created on: May 14, 2019
 *      Author: krad2
 */

#ifndef TASK_H_
#define TASK_H_

#include <msp430.h>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "context.h"

using runnable = std::uint16_t (*)(void *);

enum class taskStates {
	dead,
	sleeping,
	alive
};

class task {
public:
	task(runnable r, std::size_t priorityVal = 1, std::size_t stackSize = 4);
	~task();

	bool isComplete(void) const;
	bool isIdle(void) const;
	bool isSleeping(void) const;

	static std::uint16_t idle(void *arg);

private:
	friend class scheduler;

	#if defined (__USEMSP430X__)
		std::uint32_t *trapframe;
	#else
		std::uint16_t *trapframe;
	#endif
	std::size_t priority;
	taskStates state = taskStates::alive;
	runnable func;
	std::uint8_t *userStack;
};

#include <task.cpp>

#endif /* TASK_H_ */
