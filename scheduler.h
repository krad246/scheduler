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
#include <algorithm>
#include <cstddef>

#include "context.h"

using schedulingMethod = const task (*)(void);

class scheduler {
public:
	scheduler(schedulingMethod m, const std::initializer_list<task>& tasks);

	void start(void);

	static const task roundRobin(void);
	static const task lottery(void);
	static const task stride(void);

	static interrupt void preempt(void);

private:
	static std::size_t currProc;
	static std::vector<task> taskList;

	static std::size_t numSleeping;

	#if defined (__USEMSP430X__)
		static std::uint32_t sysSp;
	#else
		static std::uint16_t sysSp;
	#endif

	static schedulingMethod method;
};

#include <scheduler.cpp>

#endif /* SCHEDULER_H_ */
