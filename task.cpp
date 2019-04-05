/*
 * task.cpp
 *
 *  Created on: Mar 29, 2019
 *      Author: krad2
 */

#include <task.h>

Task *TaskQueue::tQueueHead = nullptr;
Task *TaskQueue::tQueueTail = nullptr;
uint8_t TaskQueue::tickSum = 0;
