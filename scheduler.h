/*
 * scheduler.h
 *
 *  Created on: May 14, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <task.h>
#include <initializer_list>
#include <vector>
#include <type_traits>
#include <cstring>

#include "context.h"

using schedulingMethod = const taskBase (*)(void);

class schedulerBase {
public:
	void start(void);

	static const taskBase roundRobin(void);
	static const taskBase lottery(void);

	static interrupt void preempt(void);

protected:
	static std::size_t currProc;
	static std::vector<taskBase> taskList;
	static std::size_t numSleeping;

	#if defined (__USEMSP430X__)
		static std::uint32_t sysSp;
	#else
		static std::uint16_t sysSp;
	#endif

	static schedulingMethod method;
};

template <schedulingMethod m = schedulerBase::roundRobin>
class scheduler : public schedulerBase {
public:
	constexpr scheduler(const std::initializer_list<taskBase>& tasks);
};

#include <scheduler.cpp>

#endif /* SCHEDULER_H_ */
