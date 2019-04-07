/*
 * Task.cpp
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#include <Task.h>

/**
 * Task constructor, creates a user stack and kernel stack and runs at a priority.
 */

Task::Task(func f, std::size_t stackSize, std::size_t priority) : priority(priority) {

	/**
	 * Kernel stack must be big enough to hold R0, R2, R4 - R15
	 */

	std::uint16_t *KernelStack = new std::uint16_t[16];

	/**
	 * We are simulating a stack on the heap, so the following layout is needed:
	 * 		HIGHER ADDRESS		REGISTER	PURPOSE
	 * 		SP[0] = [15]		PC			Function identifier / guard bytes
	 * 		SP[1] = [14]		PC			Idle hook in case function returns
	 * 		SP[2] = [13]		PC			Function address for entry; gets updated over time
	 *		SP[3] = [12]		SR			Status bits to maintain interrupt state
	 *	  	SP[4] - SP[15]		R4 - R15	General purpose register state
	 */

	/**
	 * Set the guard bytes, PC / SR values.
	 */

	KernelStack[15] = (std::uint16_t) f;
	KernelStack[14] = (std::uint16_t) Task::idle;
	KernelStack[13] = (std::uint16_t) f;
	KernelStack[12] = GIE;

	/**
	 * Set default register state.
	 */

	for (std::int16_t i = 11; i >= 0; i--) {
		KernelStack[i] = 0x0000;
	}

	/**
	 * The base address of the array is the top of the stack since
	 * the heap and stack grow the opposite way.
	 */

	KernelStackPointer = KernelStack;

	/**
	 * Enforce the 2-byte boundary rule by rounding the number of bytes to the
	 * nearest number of words greater or equal in size.
	 */

	const std::uint16_t NumWordsAllocated = stackSize > 0 ? 1 + (stackSize >> 1) : 0;

	/**
	 * Allocate a user stack, but move the stack pointer up since no allocations were made.
	 */

//	std::uint16_t *UserStack = new std::uint16_t[NumWordsAllocated];
//	UserStackPointer = UserStack + NumWordsAllocated;
}

/**
 * Destructor must delete previously allocated heap memory.
 */

Task::~Task() {
//	delete UserStackPointer;
	delete KernelStackPointer;
}

/**
 * Idle task hook. Goes into low power states for efficiency.
 */

void Task::idle(void) {
	_low_power_mode_0();
}

/**
 * Wrapper function to create and store a task with the specified attributes.
 * Requires a function pointer for the runnable and can accept a custom size stack and priority.
 */

void TaskQueue::addTask(func f, std::size_t stackSize, std::size_t priority) {

	/**
	 * Since our task queue is going to live on the system stack (the actual stack),
	 * we heap-allocate our tasks for separation and hold pointers to each task for access.
	 */

	Task *proc = new Task(f, stackSize, priority);

	/**
	 * Save the task's reference for access.
	 */

	push_back(proc);

	tickets += priority;
}
