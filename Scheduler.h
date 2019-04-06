/*
 * Scheduler.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <msp430.h>
#include <Math.h>
#include <SystemClock.h>
#include "Task.h"

class TaskQueue;
class SystemClock;

class Scheduler {
public:
	using SchedulingMethod = Task *(*)(Scheduler *);

	static Task *lottery(Scheduler *arg);
	static Task *roundRobin(Scheduler *arg);

	Scheduler(TaskQueue& tasks, SchedulingMethod method = Scheduler::roundRobin);
	~Scheduler();

	void start(std::size_t frequency = (std::size_t) 1000000);

	void sleep(std::size_t micros);

private:
	static Scheduler *sched;
	static std::uint16_t *SchedStackPointer;

	TaskQueue &queue;
	SchedulingMethod callback;

	static inline void popAutomaticallyPushedRegisters(void) {
		asm volatile ( \
			"   pop r8 \n" \
			"   pop r9 \n" \
			"	pop r10 \n" \
			"   pop r11 \n" \
			"   pop r12 \n" \
			"	pop r13 \n" \
			"	pop r14 \n" \
			"	pop r15 \n" \
		);
	}

	static inline void saveContext(void) {
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
		);
	}

	static inline void restoreContext(void) {
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
		);
	}

	static inline void jumpToNextTask(void) {
		asm volatile (
			" 	reti \n"
		);
	}

	static inline void enterKernelCode(void) {
		popAutomaticallyPushedRegisters();
		saveContext();
		_set_SP_register((std::uint16_t) Scheduler::SchedStackPointer);
	}

	static inline void exitKernelCode(Task *runnable) {
		_set_SP_register((std::uint16_t) runnable->KernelStackPointer);
		restoreContext();
		jumpToNextTask();
	}

	static inline void freeCompletedTasks(void) {
		ListIterator<Task *> TaskIterator = Scheduler::sched->queue.begin();
		for (std::size_t i = 0; i < Scheduler::sched->queue.size(); ++i) {
			if ((*TaskIterator)->complete()) {
				ListIterator<Task *> TaskToBeFreed = TaskIterator;
				TaskIterator++;
				Scheduler::sched->queue.pop(TaskToBeFreed);
			} else {
				(*TaskIterator)->KernelStackPointer[15] = (*TaskIterator)->KernelStackPointer[13];
				TaskIterator++;
			}
		}
	}

	static interrupt void preempt(void);
};

#endif /* SCHEDULER_H_ */
