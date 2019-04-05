/*
 * task.h
 *
 *  Created on: Mar 29, 2019
 *      Author: krad2
 */

/*
 * task.h
 *
 *  Created on: Mar 29, 2019
 *      Author: krad2
 */

#ifndef TASK_H_
#define TASK_H_

#include <msp430.h>
#include <stdint.h>
#include <kmath.h>

// Macro that statically allocates a task struct
#define createTask(func, stackSize, priority) \
	static vStackTask<func, stackSize, priority> handle(func, stackSize, priority)

// Macro that generates a name for the task
#define handle(func, stackSize, priority) \
	task_ ## func ## _ ## stackSize ## _ ## priority


#define saveContext() \
	asm volatile ( \
		"	push r4 \n" \
		"	push r5 \n" \
		"	push r6 \n" \
		"	push r7 \n" \
		"	push r8 \n" \
		"	push r9 \n" \
		"	push r10 \n" \
		"	push r11 \n" \
		"	push r12 \n" \
		"	push r13 \n" \
		"	push r14 \n" \
		"	push r15 \n" \
	)

#define restoreContext() \
	asm volatile ( \
		"	pop r15 \n" \
		"	pop r14 \n" \
		"	pop r13 \n" \
		"	pop r12 \n" \
		"	pop r11 \n" \
		"	pop r10 \n" \
		"	pop r9 \n" \
		"	pop r8 \n" \
		"	pop r7 \n" \
		"	pop r6 \n" \
		"	pop r5 \n" \
		"	pop r4 \n" \
	)

class taskQueue;

// Base task class, does not specify a stack size
class Task {
public:
	Task() : next(nullptr) {};

	//	An instance of the derived will contain these plus a variable length stack
	uint8_t priority;
	uint16_t *sp;

private:
	friend class TaskQueue;

	// Contains basic elements of a linked list for a feedback queue
	Task *next;
};


using proc = void (*)(void);

// Templated variable stack task, specify what to run, stack size, and priority level
template <proc p, uint16_t stackSize, uint8_t priorityLvl = 1>
class vStackTask : public Task {
public:
//	Compile time initialization of task's stack, etc
	constexpr vStackTask() : Task() {
		static_assert(stackSize >= 28, "Stack size too low to hold registers, must be at least 28 bytes.");
		this->priority = priorityLvl;
		register uint16_t *stackBase = (uint16_t *) (this->stack + (stackSize - 1));
		this->sp = stackBase;
		*this->sp-- = (uint16_t) p;
		*this->sp-- = GIE;
		for (register uint8_t i = 15; i >= 4; i--) {
			*this->sp-- = 0;
		}

		this->sp++;
	}

private:
	uint8_t stack[stackSize];
};

// Wrapper feedback queue class
class TaskQueue {
public:
//	Compile time initialization of feedback queue as a linked list
	template <proc p, uint16_t stackSize = 29, uint8_t priorityLvl = 1>
	static inline void addTask(void) {
//		Creates a task with a unique identifier, statically allocated
		createTask(p, stackSize, priorityLvl);
		tickSum += priorityLvl;

//		Linked list logic
		if (tQueueHead == nullptr) {
			tQueueHead = &handle(p, stackSize, priorityLvl);
			tQueueTail = tQueueHead;
		} else {
			register Task *end = tQueueTail;

			end->next = &handle(p, stackSize, priorityLvl);
			tQueueTail = end->next;
		}
	}

	static const Task &pickTask(void) {
		const uint16_t randVal = xs16();
		const uint16_t tickPow2Sum = nearestPow2(tickSum);
		const uint16_t loc = randVal & (tickPow2Sum - 1);

		uint16_t lo = 0;
		uint16_t hi = 0;
		Task *iter = tQueueHead;
		while (iter != tQueueTail) {
			hi += iter->priority;
			if (lo <= loc && loc < hi) break;
			iter = iter->next;
			lo += iter->priority;
		}

		return *iter;
	}

private:
	static Task *tQueueHead;
	static Task *tQueueTail;
	static uint8_t tickSum;

};

using tq = TaskQueue;

#endif /* TASK_H_ */
