/*
 * softTimer.h
 *
 *  Created on: Oct 12, 2025
 *      Author: mtri1
 */

/* timer.h */
#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "global.h"

// Định nghĩa các Timer ID
#define TIMER_TRAFFIC   0  // Dùng cho đếm ngược đèn giao thông
#define TIMER_BLINK     1  // Dùng cho nháy LED (nếu cần)
#define TIMER_SCAN      2  // Dùng cho quét 7SEG (nếu cần)

#define NO_OF_TIMERS    3
#define TIMER_CYCLE     10 // Chu kỳ gọi timerRun (10ms)

extern int timer_counter[NO_OF_TIMERS];
extern int timer_flag[NO_OF_TIMERS];

void setTimer(int index, int duration);
void timerRun();

#endif /* INC_TIMER_H_ */
