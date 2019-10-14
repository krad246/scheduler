/*
 * scheduler_base.h
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_BASE_H_
#define SCHEDULER_BASE_H_

#include <task.h>
#include <config.h>
#include <stable_priority_queue.h>
#include <ring_buffer.h>

#include <cstdlib>
#include <cstddef>

#include <algorithm>
#include <vector>
#include <utility>
#include <initializer_list>
#include <queue>
#include <unordered_map>

/**
 * Base instance of all schedulers - all schedulers share this
 */

using isr = void (*)(void);

class abstract_scheduler {
public:
	// Scheduler tick interrupt
	static __attribute__((interrupt)) void preempt(void);

	void attach_interrupt(void (*isr)(void), const task &driver_func);
	void schedule_interrupt(void (*isr)(void));
	void service_interrupts(void);

protected:
	abstract_scheduler();

	// Pointer to current process
	task *current_process = nullptr;

	// Pointer to top of OS-reserved stack
#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
	std::uint32_t kstack_ptr = 0x0000;
#else
	std::uint16_t kstack_ptr = 0x0000;
#endif

	// Queue of interrupts waiting to be scheduled
	ring_buffer<isr> isr_wait_queue;

	// Queue of interrupts scheduled (FIFO with priority sorting)
	stable_priority_queue<task> isr_sched_queue;

	// Hash table mapping interrupt to associated task
	std::unordered_map<isr, task> isr_vec_table;
};

/**
 * Specialization of abstract scheduler with compile-time member variable selection depending on
 * template argument (selection of scheduling algorithm)
 */

template <scheduling_algorithms alg> class base_scheduler { };

/**
 * Round robin scheduler instance - implements weighted round-robin algorithm
 */

template <>
class base_scheduler<scheduling_algorithms::round_robin> : public abstract_scheduler {
public:

	/**
	 * Constructors to initialize member variables using an initializer list of tasks, etc.
	 */

	base_scheduler();
	base_scheduler(const std::initializer_list<task> &task_list);


	// Adds / removes task to / from process queue
	void add_task(const task &t);
	void cleanup(const task &t);

	// Starts OS up once initialized correctly
	void start(void);

	// Schedules a process
	task &schedule(void);

public:
	// List of tasks and pointer to current task & associated run counter in list
	std::vector<std::pair<task, std::uint8_t>> tasks;
	std::vector<std::pair<task, std::uint8_t>>::iterator current_task_ptr;
};

/**
 * Lottery scheduler implementation of scheduler
 */

template <>
class base_scheduler<scheduling_algorithms::lottery> : public abstract_scheduler {
private:

	task &schedule(void);
};


#endif /* SCHEDULER_BASE_H_ */
