/*
 * scheduler.cpp
 *
 *  Created on: Sep 11, 2019
 *      Author: krad2
 */

#include <scheduler.h>

task &base_scheduler<scheduling_algorithms::round_robin>::schedule(void) {
	std::vector<std::pair<task, std::uint8_t>>::iterator &task_ptr = this->current_task_ptr;
	std::vector<std::pair<task, std::uint8_t>> &tasks = this->tasks;

	std::size_t num_avail = tasks.size();

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

base_scheduler<scheduling_algorithms::round_robin>::base_scheduler() {

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

