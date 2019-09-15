
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

enum class scheduling_algorithms {
	round_robin,
	lottery
};

using isr = void (*)(void);

struct isr_mailbox_retrieval {
	std::unordered_map<isr, std::queue<message>>::iterator mailbox_ptr;
	std::int16_t ret_code;
};

class abstract_scheduler {
public:
	task *current_process = nullptr;
	std::uint16_t kstack_ptr = 0x0000;

	std::unordered_map<isr, std::queue<message>> isr_message_table = std::unordered_map<isr, std::queue<message>>();
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
	scheduler();
	scheduler(const std::initializer_list<task> &task_list);

	void start(void);
	void start(const std::initializer_list<task> &task_list);

	inline void enter_kernel_mode(void);
	inline void leave_kernel_mode(void);

	inline void enter_kstack(void);
	inline void leave_kstack(void);

	task &schedule(void);

	struct isr_mailbox_retrieval fetch_mailbox(isr caller);

	inline task &get_current_process(void);
	inline void sleep(std::size_t ticks);
	inline void block(void);
	inline void unblock(void);
//	void sleep(std::size_t ticks);
};

template <scheduling_algorithms alg>
scheduler<alg>::scheduler() {
	this->kstack_ptr = 0x0000;
}

template <scheduling_algorithms alg>
scheduler<alg>::scheduler(const std::initializer_list<task> &task_list) : base_scheduler<alg>(task_list) {
	this->kstack_ptr = _get_SP_register();
}

template <scheduling_algorithms alg>
inline task &scheduler<alg>::get_current_process(void) {
	return *this->current_process;
}

extern interrupt void USCI_A1_ISR(void);

template <scheduling_algorithms alg>
void scheduler<alg>::start(void) {
	if (this->tasks.size() == 0) {
		// halt
	}

	WDTCTL = WDT_ADLY_16;
	SFRIE1 |= WDTIE;

	this->isr_message_table.insert(std::make_pair<isr, std::queue<message>>(reinterpret_cast<isr>(USCI_A1_ISR), std::queue<message>()));
	this->schedule().load();
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
inline void scheduler<alg>::enter_kstack(void) {
	_set_SP_register(this->kstack_ptr);
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::leave_kstack(void) {
	_set_SP_register(this->get_current_process().get_latest_sp());
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
	_low_power_mode_0();

	// to optimize, instead of calling lpm0,
	// push 2 dummy words then call the scheduler

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
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::block(void) {
	this->current_process->block();
	_low_power_mode_0();

	// to optimize, instead of calling lpm0,
	// push 2 dummy words then call the scheduler

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
}

template <scheduling_algorithms alg>
inline void scheduler<alg>::unblock(void) {
	this->current_process->unblock();
}
template <scheduling_algorithms alg>
struct isr_mailbox_retrieval scheduler<alg>::fetch_mailbox(isr caller) {
	std::unordered_map<isr, std::queue<message>>::iterator mailbox = this->isr_message_table.find(caller);
	return (struct isr_mailbox_retrieval) {
		.mailbox_ptr = mailbox,
		.ret_code = (mailbox == this->isr_message_table.end() ? 0 : -1)
	};
}


#endif /* SCHEDULER_H_ */
