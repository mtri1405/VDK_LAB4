/* global.h */
#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "main.h"
#include "timer.h"
#include "button.h"
#include "led.h"
#include "7_SEGMENT.h"
#include "scheduler.h"
#include "fsm_auto.h"
#include "fsm_manual.h"

// --- Định nghĩa các chế độ hoạt động (STATUS) ---
#define INIT        0
#define AUTO        1
#define MAN_RED     2
#define MAN_GREEN   3
#define MAN_AMBER   4

// --- Định nghĩa màu đèn (Index mảng) ---
#define RED_IDX     0
#define GREEN_IDX   1
#define AMBER_IDX   2

// --- Biến toàn cục ---
extern int STATUS;
extern int TrafficTimer[3]; // Thời gian gốc: RED, GREEN, AMBER

// Hàm điều phối chính
void run();

#endif /* INC_GLOBAL_H_ */
