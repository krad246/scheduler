/**
 * Task.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef TASK_H_
#define TASK_H_

#include <msp430.h>
#include <cstddef>
#include <cstdint>
#include <List.h>

/**
 * Task class:
 * Represents basic unit of execution in the system.
 */

/**
 * Function signature for the runnables.
 */

using func = void(*)(void);

class Task {
public:

	/**
	 * Task constructor, creates a user stack and kernel stack and runs at a priority.
	 */

	Task(func f, std::size_t stackSize = 0, std::size_t priority = 1);

	/**
	 * Deallocate the heap memory associated with the task.
	 */

	~Task();

	/**
	 * If a function completes, then the system will return and pop an additional word off the stack.
	 * When re-entering the function, the status register will overwrite the return address.
	 * Since the status register is 1 byte large, it occupies the lower byte of the word.
	 * Then we know that iff the word is fully contained in a byte then it must be the SR.
	 * So we test if the higher byte is fully zeroed out, as it must be.
	 */

	inline bool complete(void) {
		const std::uint16_t statusWord = KernelStackPointer[13];
		return (statusWord & 0xFF00) == 0x00;
	}

	/**
	 * Idle task hook.
	 */

	static void idle(void);

private:
	friend class Scheduler;

	std::uint16_t *Stack;
	std::uint16_t *KernelStackPointer;

	std::size_t priority;

	bool sleeping = false;
	std::size_t sleepMicros = 0;
	std::size_t sleepMillis = 0;
};


/**
 * Linked list of tasks for dynamic and controlled growth / shrinkage.
 * Contains pointers to the heap-allocated tasks to prevent scoping issues.
 */

class TaskQueue : List<Task *> {
public:

	/**
	 * Wrapper function to push a task with the specified parameters to the
	 * underlying linked list.
	 */

	void addTask(func f, std::size_t stackSize = 0, std::size_t priority = 1);

private:
	friend class Scheduler;

	std::size_t numSleeping = 0;
	std::size_t tickets = 0;
};

#endif /* TASK_H_ */
