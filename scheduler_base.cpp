/*
 * scheduler_base.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#include <scheduler_base.h>
#include <scheduler.h>

extern scheduler<scheduling_algorithms::lottery> os;

/**
 * Default constructor
 */

abstract_scheduler::abstract_scheduler() {
	this->isr_wait_queue = ring_buffer<isr>(INT_QUEUE_SIZE);
}

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
	this->isr_wait_queue.put(isr);	// Simple numeric copy is faster
}

/**
 * Handles interrupt decision-making in the scheduler
 */

void abstract_scheduler::service_interrupts(void) {
	// If any interrupt handlers have completed in some way then remove them
	while (this->isr_sched_queue.size() > 0 && this->isr_sched_queue.top().blocking()) {
		this->isr_sched_queue.pop();
	}

	while (!this->isr_wait_queue.empty()) {	// If any new interrupts were receieved, push the corresponding tasks to the scheduler queue
		auto driver_ptr = this->isr_vec_table.find(this->isr_wait_queue.get());	// Find the task corresponding to the ISR just executed
		this->isr_sched_queue.emplace(driver_ptr->second);	// Push it to the waiting queue
	}
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
 * Initializes the scheduler with an empty set of tasks, etc.
 */

base_scheduler<scheduling_algorithms::lottery>::base_scheduler() {
	this->tasks = std::vector<task>();
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
 * Initializes all of the passed-in tasks, places them in the task list, and sets the start pointer
 */

base_scheduler<scheduling_algorithms::lottery>::base_scheduler(const std::initializer_list<task> &task_list) {
	std::transform (
		task_list.begin(),
		task_list.end(),
		std::back_inserter(this->tasks),
		[] (const task &t) {
			return t;
		}
	);
}

/**
 * Adds a task to the list
 */

void base_scheduler<scheduling_algorithms::round_robin>::add_task(const task &t) {
	this->tasks.emplace_back(std::move(std::make_pair(t, t.get_priority())));
}

/**
 * Adds a task to the list
 */

void base_scheduler<scheduling_algorithms::lottery>::add_task(const task &t) {
	this->tasks.emplace_back(std::move(t));
}

/**
 * Deletes a completed task
 */

void base_scheduler<scheduling_algorithms::round_robin>::cleanup(const task &t) {
	auto it = std::remove_if(
		this->tasks.begin(),
		this->tasks.end(),
		[&](const std::pair<task, std::uint8_t>& element) {
			return element.first == t;
		}
	);

	if (it == this->tasks.end()) return;
	this->tasks.erase(it);
}

/**
 * Deletes a completed task
 */

void base_scheduler<scheduling_algorithms::lottery>::cleanup(const task &t) {
	auto it = std::remove_if(					// Find the task to delete and move it to the end so it can be deleted cleanly
		this->tasks.begin(),
		this->tasks.end(),
		[&](const task& element) {
			return element == t;
		}
	);

	if (it == this->tasks.end()) return;		// If it doesn't exist then there is a problem
	this->tasks.erase(it);						// Else erase the task
}

/**
 * Sets the system stack up, sets the start pointer
 */

void base_scheduler<scheduling_algorithms::round_robin>::start(void) {
	this->current_task_ptr = tasks.begin();
	this->kstack_ptr = _get_SP_register();
}

/**
 * Sets the system stack up, sets the start pointer
 */

void base_scheduler<scheduling_algorithms::lottery>::start(void) {
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

	std::size_t num_avail = this->num_tasks;
	if (num_avail == 0) {
		this->current_process = &task::idle_hook;
		return task::idle_hook;
	}

	/**
	 * Update the sleep state of every task before considering what can be scheduled
	 */

	for (auto it = tasks.begin(); it < tasks.end(); ++it) {
		std::pair<task, std::uint8_t> &proc = *it;
		task &t = proc.first;
		t.update();
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
			proc.second = t.get_priority();
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

/**
 * Fast clz using Debruijn multiplication
 */

std::uint32_t clz(std::uint32_t x) {
    static const char debruijn32[32] = {
        0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
        1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18
    };

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return debruijn32[x * 0x076be629 >> 27];
}

/**
 * XORShift random number generator for various widths. The period of a XORShift generator with width N
 * is -1 + (1 << N). For more randomness, use a bigger one. The tradeoff is performance.
 */

inline std::uint16_t rand16(void) {
	static std::uint16_t state = 1;
    state ^= state << 7;
    state ^= state >> 9;
    state ^= state << 8;
    return state;
}

inline std::uint32_t rand32(void) {
	static std::uint32_t state = 2463534242;
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

inline std::uint64_t rand64(void) {
	static std::uint64_t state = 8817264546332525;
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

/**
 * Fast random calculation avoiding modulo division by constraining to a range of powers of 2
 */

std::uint32_t bounded_rand32(std::uint32_t (*rng)(void), std::uint32_t range) {

	/**
	 * Get the first power of 2 exceeding the range
	 */
    std::uint32_t mask = ~std::uint32_t(0);
    --range;
    mask >>= clz(range | 1);

    /**
     * Perform modulo within the range through bitwise AND, but delete the numbers not in the range
     */

    std::uint32_t x;
    do {
        x = rng() & mask;
    } while (x > range);

    return x;
}

task &base_scheduler<scheduling_algorithms::lottery>::schedule(void) {

	/**
	 * If there are no tasks available, simply idle
	 */

	std::size_t num_avail = this->num_tasks;
	if (num_avail == 0) {
		this->current_process = &task::idle_hook;
		return task::idle_hook;
	}

	/**
	 * Update the sleep state of every task before considering what can be scheduled
	 * We're also combining this with a calculation of the intervals since both are O(n)
	 */

	std::vector<std::uint16_t> intervals;				// Reserve a vector of intervals
	intervals.reserve(num_avail);

	std::uint16_t left = 0;
	for (auto it = this->tasks.begin(); it < this->tasks.end(); ++it) {	// Loop through, check status and update
		task &t = *it;
		t.update();

		auto pri = t.get_priority();						// If the task is eligible to be scheduled, add the interval
		if (!t.sleeping() && !t.blocking()) left += pri;
		intervals.push_back(left);
	}

	const auto pool_size = left;
	if (left == 0) {	// Check if any tasks are eligible
		this->current_process = &task::idle_hook;
		return task::idle_hook;
	}

	const auto roll = bounded_rand32(rand32, pool_size);	// Compute fast random modulus for the draw
	auto it = std::upper_bound(intervals.begin(), intervals.end(), roll);	// Binary search to find the process
	auto idx = it - intervals.begin();

	this->current_process = this->tasks.data() + idx;		// Update and return
	return this->tasks[idx];
}

/**
 * Scheduler tick performs a context switch
 */

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void abstract_scheduler::preempt(void) {
	os.context_switch();
}
