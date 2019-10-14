/*
 * config.cpp
 *
 *  Created on: Sep 18, 2019
 *      Author: krad2
 */

#include <config.h>
#include <scheduler.h>

/**
 * Select the scheduler and the type of scheduling algorithm to use
 */

scheduler<scheduling_algorithms::round_robin> os;
