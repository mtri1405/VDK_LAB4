/*
 * scheduler.h
 *
 *  Created on: Nov 11, 2025
 *      Author: MinhTri
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>
#include "main.h"

#define SCH_MAX_TASKS 20
#define RETURN_NORMAL 0
#define RETURN_ERROR 1

typedef struct {
    void (*pTask)(void);  // Con trỏ hàm thực thi tác vụ
    uint16_t Delay;       // Thời gian chờ trước khi chạy
    uint16_t Period;      // Chu kỳ lặp lại (ms)
    uint8_t RunMe;        // Cờ đánh dấu tác vụ sẵn sàng chạy
} sTask;

// Biến toàn cục cho scheduler
extern sTask SCH_tasks_G[SCH_MAX_TASKS];

// Hàng đợi cho các tác vụ sẵn sàng
// Biến toàn cục
extern uint8_t ReadyQueue[];      // Lưu chỉ số các tác vụ cần chạy
extern uint8_t ReadyQueueSize;    // Kích thước hàng đợi

// Các hàm API
void SCH_Insert_Into_Queue(uint8_t taskIndex);
uint8_t SCH_Remove_From_Queue();
void SCH_Init(void);
uint8_t SCH_Add_Task(void (*pFunction)(), uint16_t DELAY, uint16_t PERIOD);
void SCH_Delete_Task(uint8_t index);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
void SCH_Go_To_Sleep(void);
#endif /* INC_SCHEDULER_H_ */
