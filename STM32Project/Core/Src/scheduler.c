#include "scheduler.h"

// Biến toàn cục
sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t ReadyQueue[SCH_MAX_TASKS]; // Lưu chỉ số các tác vụ cần chạy
uint8_t ReadyQueueSize = 0;        // Kích thước hàng đợi

void SCH_Insert_Into_Queue(uint8_t taskIndex) {
    uint8_t i = 0;
    // Điều chỉnh delay tương đối
    for (i = 0; i < ReadyQueueSize; i++) {
        if (SCH_tasks_G[taskIndex].Delay < SCH_tasks_G[ReadyQueue[i]].Delay) {
            SCH_tasks_G[ReadyQueue[i]].Delay -= SCH_tasks_G[taskIndex].Delay;
            break;
        }
        SCH_tasks_G[taskIndex].Delay -= SCH_tasks_G[ReadyQueue[i]].Delay;
    }
    // Dịch chuyển các phần tử
    for (uint8_t j = ReadyQueueSize; j > i; j--) {
        ReadyQueue[j] = ReadyQueue[j - 1];
    }
    // Thêm tác vụ
    ReadyQueue[i] = taskIndex;
    ReadyQueueSize++;
}

// Hàm loại bỏ tác vụ khỏi đầu hàng đợi (Đã chuẩn)
uint8_t SCH_Remove_From_Queue() {
    if (ReadyQueueSize == 0) {
        return SCH_MAX_TASKS; // Hàng đợi trống
    }
    uint8_t taskIndex = ReadyQueue[0];
    if (ReadyQueueSize > 1) {
        SCH_tasks_G[ReadyQueue[1]].Delay += SCH_tasks_G[taskIndex].Delay;
    }
    for (uint8_t i = 1; i < ReadyQueueSize; i++) {
        ReadyQueue[i - 1] = ReadyQueue[i];
    }
    ReadyQueueSize--;
    return taskIndex;
}

// Hàm khởi tạo
void SCH_Init(void) {
    ReadyQueueSize = 0;
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        // Chỉ cần xóa mảng task chính
        SCH_tasks_G[i].pTask = NULL;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
    }
}


uint8_t SCH_Add_Task(void (*pFunction)(), uint16_t DELAY, uint16_t PERIOD) {
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (!SCH_tasks_G[i].pTask) {  // Tìm vị trí trống
            SCH_tasks_G[i].pTask = pFunction;
            SCH_tasks_G[i].Delay = DELAY; // tick 10ms
            SCH_tasks_G[i].Period = PERIOD;
            SCH_tasks_G[i].RunMe = 0;

            SCH_Insert_Into_Queue(i);
            return i;  // Trả về chỉ số tác vụ
        }
    }
    return SCH_MAX_TASKS;  // Hết chỗ trống
}


void SCH_Delete_Task(uint8_t taskIndex) {
    if (taskIndex >= SCH_MAX_TASKS || SCH_tasks_G[taskIndex].pTask == NULL) {
        return; // Task không tồn tại hoặc index sai
    }

    // 1. Tìm task trong ReadyQueue
    uint8_t i = 0;
    for (i = 0; i < ReadyQueueSize; i++) {
        if (ReadyQueue[i] == taskIndex) {
            break; // Tìm thấy tại vị trí i
        }
    }

    // 2. Nếu tìm thấy trong queue, xóa nó
    if (i < ReadyQueueSize) {
        // Nếu không phải task cuối, dồn delay cho task sau
        if (i < ReadyQueueSize - 1) {
            SCH_tasks_G[ReadyQueue[i+1]].Delay += SCH_tasks_G[ReadyQueue[i]].Delay;
        }

        // Dịch chuyển queue
        for (uint8_t j = i; j < ReadyQueueSize - 1; j++) {
            ReadyQueue[j] = ReadyQueue[j+1];
        }
        ReadyQueueSize--;
    }

    // 3. Xóa khỏi mảng task chính
    SCH_tasks_G[taskIndex].pTask = NULL;
    SCH_tasks_G[taskIndex].Delay = 0;
    SCH_tasks_G[taskIndex].Period = 0;
    SCH_tasks_G[taskIndex].RunMe = 0;
}

void SCH_Update(void) {
    if (ReadyQueueSize == 0) return;

    // Chỉ giảm Delay của tác vụ đầu tiên trong hàng đợi
    // Vì các tác vụ sau lưu Delay tương đối so với tác vụ trước
    if (SCH_tasks_G[ReadyQueue[0]].Delay > 0) {
        SCH_tasks_G[ReadyQueue[0]].Delay--;
    }

    // --- SỬA ĐỔI: Dùng vòng lặp để xử lý TẤT CẢ tác vụ đã đáo hạn (Delay == 0) ---
    while (ReadyQueueSize > 0 && SCH_tasks_G[ReadyQueue[0]].Delay == 0) {
        // Lấy tác vụ đầu tiên
        uint8_t taskIndex = ReadyQueue[0];

        // Đánh dấu tác vụ cần chạy
        SCH_tasks_G[taskIndex].RunMe += 1;

        // Loại bỏ khỏi hàng đợi hiện tại
        SCH_Remove_From_Queue();

        // Nếu là tác vụ định kỳ, nạp lại vào hàng đợi
        if (SCH_tasks_G[taskIndex].Period > 0) {
            SCH_tasks_G[taskIndex].Delay = SCH_tasks_G[taskIndex].Period;
            SCH_Insert_Into_Queue(taskIndex);
        }
    }
}
void SCH_Dispatch_Tasks(void) {
	for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
		if (SCH_tasks_G[i].RunMe > 0) {
			// Chạy tác vụ tại chỉ số i
			(*SCH_tasks_G[i].pTask)();
			SCH_tasks_G[i].RunMe -=1; // Giảm cờ

			// Nếu là tác vụ chạy 1 lần, xóa nó
			if (SCH_tasks_G[i].Period == 0) {
				SCH_Delete_Task(i);
			}
		}
	}
}
void SCH_Go_To_Sleep(void){
	__WFI();
}
