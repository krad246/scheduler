/*
 * print.h
 *
 *  Created on: Feb 8, 2019
 *      Author: krad2
 */

#ifndef PRINT_H_
#define PRINT_H_

#include <scheduler.h>

#include <msp430.h>
#include <cstdarg>

void putc(unsigned);
void puts(char *);
void send_byte(unsigned char byte);
void print(char *format, ...);

void initUART();

#endif /* PRINT_H_ */
