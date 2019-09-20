/*
 * scheduler_base.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#include <scheduler_base.h>
#include <scheduler.h>

extern scheduler<scheduling_algorithms::round_robin> os;

/**
 * Creates an interrupt handler task for the scheduler, mapped to an interrupt or other callback
 */

void abstract_scheduler::attach_interrupt(void (*isr)(void), const task &driver_func) {
	this->isr_vec_table.emplace(std::make_pair(isr, driver_func));
}

/**
 * Pushes a caught interrupt on the ISR wait queue for future handling
 */

void abstract_scheduler::schedule_interrupt(void (*isr)(void)) {
	auto driver_ptr = this->isr_vec_table.find(isr);	// Find the task corresponding to the ISR just executed
	this->isr_wait_queue.push_back(driver_ptr->second);	// Push it to the waiting queue
	std::push_heap(this->isr_wait_queue.begin(), this->isr_wait_queue.end(), cmp_isr_deadline());	// Sort by earliest deadline
}

/**
 * Initializes the scheduler with an empty set of tasks, etc.
 */

base_scheduler<scheduling_algorithms::round_robin>::base_scheduler() {
	this->tasks = std::vector<std::pair<task, std::uint8_t>>();
	this->current_task_ptr = tasks.begin();
	this->kstack_ptr = 0x0000;
}

/**
 * Initializes all of the passed-in tasks, places them in the task list, and sets the start pointer
 */

base_scheduler<scheduling_algorithms::round_robin>::base_scheduler(const std::initializer_list<task> &task_list) {
	std::transform (
		task_list.begin(),
		task_list.end(),
		std::back_inserter(this->tasks),
		[] (const task &t) {
			return std::make_pair(t, t.get_priority());
		}
	);

	this->current_task_ptr = tasks.begin();
}

/**
 * Adds a task to the list
 */

void base_scheduler<scheduling_algorithms::round_robin>::add_task(const task &t) {
	this->tasks.emplace_back(std::move(std::make_pair(t, t.get_priority())));
}

/**
 * Deletes a completed task
 */

void base_scheduler<scheduling_algorithms::round_robin>::cleanup(task &t) {
	(void) t;

}

/**
 * Sets the system stack up, sets the start pointer
 */

void base_scheduler<scheduling_algorithms::round_robin>::start(void) {
	this->current_task_ptr = tasks.begin();
	this->kstack_ptr = _get_SP_register();
}

/**
 * Implements a weighted round-robin scheduler
 */

task &base_scheduler<scheduling_algorithms::round_robin>::schedule(void) {
	std::vector<std::pair<task, std::uint8_t>>::iterator &task_ptr = this->current_task_ptr;
	std::vector<std::pair<task, std::uint8_t>> &tasks = this->tasks;

	/**
	 * If there are no tasks available, simply idle
	 */

	std::size_t num_avail = tasks.size();
	if (num_avail == 0) {
		this->current_process = &task::idle_hook;
		return task::idle_hook;
	}

	/**
	 * Otherwise, iterate sequentially, paying attention to the weights
	 */

	for (;;) {

		/**
		 * For each slot in the list of tasks:
		 * - Check if it is blocking / sleeping - if so, it is not available
		 * 	 Also update its sleep counter for this tick
		 * - Otherwise, check if it still needs to be run
		 *   If so, decrement the run counter and return the task
		 *   Otherwise, refresh its run counter and move to the next task
		 */

		std::pair<task, std::uint8_t> &proc = *task_ptr;
		task &t = proc.first;
		const std::size_t run_counts_remaining = proc.second;

		if (!t.sleeping() && !t.blocking()) {
			if (run_counts_remaining > 0) {
				proc.second--;
				break;
			} else {
				proc.second = t.get_priority();
			}
		} else {
			num_avail--;
			t.update();
		}

		/**
		 * If no tasks are available as a result of sleeping / blocking, return the idle hook
		 */

		if (num_avail == 0) {
			this->current_process = &task::idle_hook;
			return task::idle_hook;
		}

		task_ptr++;
		if (task_ptr == tasks.end()) task_ptr = tasks.begin();
	}

	this->current_process = &task_ptr->first;
	return task_ptr->first;
}

task &base_scheduler<scheduling_algorithms::lottery>::schedule(void) {
	for (;;) {

	}
}

/**
 * Scheduler tick performs a context switch
 */

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void abstract_scheduler::preempt(void) {
	os.context_switch();
}
