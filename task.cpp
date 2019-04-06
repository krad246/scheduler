/*
 * Task.cpp
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#include <Task.h>

Task::Task() : UserStackPointer(nullptr), KernelStackPointer(nullptr), priority(1) {}

Task::Task(func f, std::size_t stackSize, std::size_t priority) : priority(priority) {
	std::uint16_t *KernelStack = new std::uint16_t[16];

	KernelStack[15] = (std::uint16_t) f;
	KernelStack[14] = (std::uint16_t) Task::idle;
	KernelStack[13] = (std::uint16_t) f;
	KernelStack[12] = GIE;

	for (std::int16_t i = 11; i >= 0; i--) {
		KernelStack[i] = 0x0000;
	}

	KernelStackPointer = KernelStack;

	const std::uint16_t NumWordsAllocated = stackSize > 0 ? 1 + (stackSize >> 1) : 0;

	std::uint16_t *UserStack = new std::uint16_t[NumWordsAllocated];
	std::fill_n(UserStack, NumWordsAllocated, 0x0000);

	UserStackPointer = UserStack + NumWordsAllocated;
//	KernelStackPointer[12] = (std::uint16_t) UserStackPointer;
}

Task::~Task() {
	delete UserStackPointer;
	delete KernelStackPointer;
}

void *Task::idle(void *arg) {
	_low_power_mode_0();
//	while (1);
	return 0;
}

void TaskQueue::addTask(func f, std::size_t stackSize, std::size_t priority) {
	Task *proc = new Task(f, stackSize, priority);
	push_back(proc);
}
