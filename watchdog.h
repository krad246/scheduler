/*
 * watchdog.h
 *
 *  Created on: Oct 14, 2019
 *      Author: krad2
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <msp430.h>
#include <config.h>

void watchdog_init(void);
void watchdog_request(void);
void watchdog_reload(void);

extern "C" void wdt_reload(void);

#endif /* WATCHDOG_H_ */
