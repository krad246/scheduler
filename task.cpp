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
	 * Enforce the 2-byte boundary rule by rounding the number of bytes to the
	 * nearest number of words greater or equal in size.
	 */

	const std::uint16_t NumWordsAllocated = 2 + (stackSize > 0 ? 1 + (stackSize >> 1) : 0);
//	const std::uint16_t NumWordsAllocated = (stackSize > 0 ? 1 + (stackSize >> 1) : 0);

	/**
	 * Kernel stack must be big enough to hold R0, R2, R4 - R15. The rest goes to the user stack.
	 */

	Stack = new std::uint16_t[16 + NumWordsAllocated];

	/**
	 * Must advance the kernel stack pointer from the base of the stack to the top of the kernel stack.
	 */

	KernelStackPointer = Stack + NumWordsAllocated;
	UserStackPointer = KernelStackPointer;

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

	KernelStackPointer[15] = (std::uint16_t) f;
	KernelStackPointer[14] = (std::uint16_t) Task::idle;
	KernelStackPointer[13] = (std::uint16_t) f;
	KernelStackPointer[12] = GIE;
	KernelStackPointer[11] = (std::uint16_t) UserStackPointer;

	/**
	 * Set default register state.
	 */

	for (std::int16_t i = 10; i >= 0; i--) {
		KernelStackPointer[i] = 0x0000;
	}
}

/**
 * Destructor must delete previously allocated heap memory.
 */

Task::~Task() {
	delete[] Stack;
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
}
