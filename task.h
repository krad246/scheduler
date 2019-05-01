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
	 * Checks if a task is complete.
	 */

	bool complete(void);

	/**
	 * Checks if a task is the idle() hook.
	 */
	bool isIdle(void);

	/**
	 * Idle task hook.
	 */

	static void idle(void);

private:
	friend class Scheduler;

	std::uint16_t *Stack;
	std::uint16_t *UserStackPointer;
	std::uint16_t *KernelStackPointer;

	std::size_t priority;

	bool sleeping = false;
	std::size_t duration = 0;
	std::size_t timeStamp = 0;
};

/**
 * If a function completes, then the system will return and pop an additional word off the stack.
 * When re-entering the function, the status register will overwrite the return address.
 * Since the status register is 1 byte large, it occupies the lower byte of the word.
 * Then we know that iff the word is fully contained in a byte then it must be the SR.
 * So we test if the higher byte is fully zeroed out, as it must be.
 */

#pragma FUNC_ALWAYS_INLINE
inline bool Task::complete(void) {
	const std::uint16_t statusWord = KernelStackPointer[13];
	return (statusWord & 0xFF00) == 0x00;
}

/**
 * We check if the return address of the function held in the kernel stack is the same as that of the idle task.
 */

#pragma FUNC_ALWAYS_INLINE
inline bool Task::isIdle(void) {
	const std::uint16_t retAddress = KernelStackPointer[15];
	return (retAddress == (std::uint16_t) Task::idle);
}

/**
 * Linked list of tasks for dynamic and controlled growth / shrinkage.
 * Contains pointers to the heap-allocated tasks to prevent scoping issues.
 */

class TaskIterator : ListIterator<Task *> {

};

class TaskQueue : List<Task *> {
public:

	/**
	 * Wrapper function to push a task with the specified parameters to the
	 * underlying linked list.
	 */

	void addTask(func f, std::size_t stackSize = 0, std::size_t priority = 1);

private:
	friend class Scheduler;
	friend class TaskIterator;

};

#endif /* TASK_H_ */
