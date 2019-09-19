
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
#include <unordered_map>
#include <config.h>

class abstract_scheduler {
public:
	task *current_process = nullptr;
	std::uint16_t kstack_ptr = 0x0000;
};

template <scheduling_algorithms alg>
class base_scheduler : protected abstract_scheduler {

};

template <>
class base_scheduler<scheduling_algorithms::round_robin> : protected abstract_scheduler {
public:
	base_scheduler();
	base_scheduler(const std::initializer_list<task> &task_list);

	void add_task(const task &t);
	void start(void);

	std::vector<std::pair<task, std::uint8_t>> tasks;
	std::vector<std::pair<task, std::uint8_t>>::iterator current_task_ptr;

	task &schedule(void);
	void cleanup(task &t);
};

template <>
class base_scheduler<scheduling_algorithms::lottery> : protected abstract_scheduler {
private:

	task &schedule(void);
};

template <scheduling_algorithms alg>
class scheduler : protected base_scheduler<alg> {
public:
	scheduler();
	scheduler(const std::initializer_list<task> &task_list);

	void add_task(const task &t);

	void init(void);
	void start(void);
	void start(const std::initializer_list<task> &task_list);

	inline void enter_kernel_mode(void);
	inline void leave_kernel_mode(void);

	inline void enter_kstack(void);
	inline void leave_kstack(void);

	task &schedule(void);

	inline task &get_current_process(void);
	const thread_info &get_thread_state(void);

	__attribute__((noinline)) void sleep(std::size_t ticks);
	__attribute__((noinline)) void block(void);
	inline void unblock(void);

	void cleanup(task &t);
};

template <scheduling_algorithms alg>
scheduler<alg>::scheduler() {
	base_scheduler<alg>::base_scheduler();
}

template <scheduling_algorithms alg>
scheduler<alg>::scheduler(const std::initializer_list<task> &task_list) : base_scheduler<alg>(task_list) {
	this->kstack_ptr = _get_SP_register();
}

template <scheduling_algorithms alg>
inline task &scheduler<alg>::get_current_process(void) {
	return *this->current_process;
}

template <scheduling_algorithms alg>
const thread_info &scheduler<alg>::get_thread_state(void) {
	return this->get_current_process().get_state();
}

extern interrupt void USCI_A1_ISR(void);

template <scheduling_algorithms alg>
void scheduler<alg>::start(void) {
	if (this->tasks.size() == 0) {
		// halt
	}
	base_scheduler<alg>::start();



	WDTCTL = WDT_ADLY_16;

	task &first = this->schedule();
	SFRIE1 |= WDTIE;

	first.load();
}

template <scheduling_algorithms alg>
void scheduler<alg>::start(const std::initializer_list<task> &task_list) {
	if (task_list.size() == 0) {
		// halt
	}

	new (this) scheduler(task_list);
	this->start();
}

template <scheduling_algorithms alg>
void scheduler<alg>::add_task(const task &t) {
	base_scheduler<alg>::add_task(t);
}

extern void driver_init(void);
extern struct task_config task_cfgs[num_tasks_declared];

template <scheduling_algorithms alg>
void scheduler<alg>::init(void) {
	_disable_interrupt();

	driver_init();

	struct task_config *end_pt = task_cfgs + sizeof(task_cfgs) / sizeof(struct task_config);
	for (struct task_config *it = task_cfgs; it < end_pt; ++it) {
		this->add_task(std::move(task(it->func, it->stack_size, it->priority)));
	}
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::enter_kstack(void) {
	_set_SP_register(this->kstack_ptr);
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::leave_kstack(void) {
	_set_SP_register(this->get_current_process().get_latest_sp());
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::enter_kernel_mode(void) {
	__disable_interrupt();
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
__attribute__((noinline)) void scheduler<alg>::sleep(const std::size_t ticks) {
	__disable_interrupt();
	// sleep saves pc

	// put task to sleep
	this->current_process->sleep(ticks);

	// retrieve pc address and reformat for bra instruction
	const register std::uint16_t temp = *reinterpret_cast<std::uint16_t *>(_get_SP_register());
	*reinterpret_cast<std::uint16_t *>(_get_SP_register()) = *reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) << 12;
	*reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) = temp;

	// branch to isr and reschedule
	bob();
}

template <scheduling_algorithms alg>
__attribute__((noinline)) void scheduler<alg>::block(void) {
	// block saves pc

	// put task to sleep / block
	this->current_process->block();

	// reformat pc address
	const register std::uint16_t temp = *reinterpret_cast<std::uint16_t *>(_get_SP_register());
	*reinterpret_cast<std::uint16_t *>(_get_SP_register()) = *reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) << 12;
	*reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) = temp;

	// call scheduler
	bob();
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::unblock(void) {
	this->current_process->unblock();
}

template <scheduling_algorithms alg>
void scheduler<alg>::cleanup(task &t) {
	_disable_interrupt();

	base_scheduler<alg>::cleanup(t);
	this->leave_kernel_mode();
}

#endif /* SCHEDULER_H_ */
