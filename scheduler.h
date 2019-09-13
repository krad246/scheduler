
/*
 * scheduler.h
 *
 *  Created on: Sep 11, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <task.h>

#include <vector>
#include <type_traits>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <cstdarg>

enum class scheduling_algorithms {
	round_robin,
	lottery
};

class abstract_scheduler {
protected:
	task *current_process;
	std::uint16_t kstack_ptr;
};

template <scheduling_algorithms alg>
class base_scheduler : protected abstract_scheduler {

};

template <>
class base_scheduler<scheduling_algorithms::round_robin> : protected abstract_scheduler {
public:
	base_scheduler();
	base_scheduler(const std::initializer_list<task> &task_list);

	std::vector<std::pair<task, std::uint8_t>> tasks;
	std::vector<std::pair<task, std::uint8_t>>::iterator current_task_ptr;

	task &schedule(void);
};

template <>
class base_scheduler<scheduling_algorithms::lottery> : protected abstract_scheduler {
private:

	task &schedule(void);
};

template <scheduling_algorithms alg>
class scheduler : protected base_scheduler<alg> {
public:
	scheduler(const std::initializer_list<task> &task_list);

	void start(void);

	inline void enter_kernel_mode(void);
	inline void leave_kernel_mode(void);

	task &schedule(void);

	inline task &get_current_process(void);
	inline void sleep(std::size_t ticks);
//	void sleep(std::size_t ticks);
};

template <scheduling_algorithms alg>
scheduler<alg>::scheduler(const std::initializer_list<task> &task_list) : base_scheduler<alg>(task_list) {
	this->kstack_ptr = _get_SP_register();
}

template <scheduling_algorithms alg>
inline task &scheduler<alg>::get_current_process(void) {
	return *this->current_process;
}

template <scheduling_algorithms alg>
void scheduler<alg>::start(void) {
	this->schedule().load();
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::enter_kernel_mode(void) {
	this->get_current_process().pause();
	_set_SP_register(this->kstack_ptr);
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::leave_kernel_mode(void) {
//	this->kstack_ptr = _get_SP_register();
	this->schedule().load();
}

template <scheduling_algorithms alg>
task &scheduler<alg>::schedule(void) {
	return base_scheduler<alg>::schedule();
}

extern __attribute__((naked, interrupt)) void bob(void);

template <scheduling_algorithms alg>
inline void scheduler<alg>::sleep(const std::size_t ticks) {
	this->current_process->sleep(ticks);
	_low_power_mode_0(); // wait for interrupts - not as efficient as callign the scheduler but still a WIP on that one
}

//template <scheduling_algorithms alg>
//void scheduler<alg>::sleep(const std::size_t ticks) {
//	_disable_interrupt();
//
//	const register std::uint16_t temp = *reinterpret_cast<std::uint16_t *>(_get_SP_register());
//	*reinterpret_cast<std::uint16_t *>(_get_SP_register()) = *reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) << 12;
//	*reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) = temp;
//
//	this->current_process->sleep(ticks);
//
//	// idle task stack gets messed up if this is uncommented ?
//	// improves latency of sleep activations though
//
////	// call the scheduler
//	bob();
//}

#endif /* SCHEDULER_H_ */
