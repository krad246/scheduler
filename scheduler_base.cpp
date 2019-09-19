/*
 * scheduler_base.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#include <scheduler_base.h>
#include <scheduler.h>

base_scheduler<scheduling_algorithms::round_robin>::base_scheduler() {
	this->tasks = std::vector<std::pair<task, std::uint8_t>>();
	this->current_task_ptr = tasks.begin();
	this->kstack_ptr = 0x0000;
}

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

void base_scheduler<scheduling_algorithms::round_robin>::add_task(const task &t) {
	this->tasks.emplace_back(std::move(std::make_pair(t, t.get_priority())));
}

void base_scheduler<scheduling_algorithms::round_robin>::cleanup(task &t) {
	(void) t;

	std::vector<std::pair<task, std::uint8_t>>::iterator task_to_remove = this->current_task_ptr;
//	std::vector<std::pair<task, std::uint8_t>>::iterator new_task_ptr = std::copy(this->tasks.erase(task_to_remove));
//	this->current_task_ptr = new_task_ptr;
}

void base_scheduler<scheduling_algorithms::round_robin>::start(void) {
	this->current_task_ptr = tasks.begin();
	this->kstack_ptr = _get_SP_register();
}

task &base_scheduler<scheduling_algorithms::round_robin>::schedule(void) {
	std::vector<std::pair<task, std::uint8_t>>::iterator &task_ptr = this->current_task_ptr;
	std::vector<std::pair<task, std::uint8_t>> &tasks = this->tasks;

	std::size_t num_avail = tasks.size();

	if (num_avail == 0) {
		this->current_process = &task::idle_hook;
		return task::idle_hook;
	}

	for (;;) {
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
			t.update();
		}

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

extern scheduler<scheduling_algorithms::round_robin> os;

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void abstract_scheduler::preempt(void) {
//	os.enter_kernel_mode();
//	os.leave_kernel_mode();
//	_disable_interrupt();
	os.context_switch();
}
