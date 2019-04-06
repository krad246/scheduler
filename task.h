/*
 * Task.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef TASK_H_
#define TASK_H_

#include <msp430.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <List.h>

using func = void *(*)(void *);

class Task {
public:
	Task();
	Task(func f, std::size_t stackSize = 0, std::size_t priority = 1);
	~Task();

	inline bool complete(void) {
		return KernelStackPointer[13] == GIE;
	}

	static void *idle(void *arg);

private:
	friend class Scheduler;

	std::uint16_t *KernelStackPointer;
	std::uint16_t *UserStackPointer;

	std::size_t priority;

	bool sleeping = false;
	std::size_t sleepMicros = 0;
	std::size_t sleepMillis = 0;
};

class TaskQueue : List<Task *> {
public:
	TaskQueue();
	~TaskQueue();
	void addTask(func f, std::size_t stackSize = 0, std::size_t priority = 1);

private:
	friend class Scheduler;
};

#endif /* TASK_H_ */
