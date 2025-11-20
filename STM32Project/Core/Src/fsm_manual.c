/* fsm_manual.c */
#include "fsm_manual.h"

int temp_time = 0;

// Hàm này được gọi mỗi khi chuyển mode (Auto -> ManRed -> ManGreen...)
void init_fsm_manual() {
    turn_off_all();
    setTimer(TIMER_BLINK, 500); // Nháy 0.5s

    switch (STATUS) {
        case MAN_RED:
            temp_time = TrafficTimer[RED_IDX];
            break;
        case MAN_GREEN:
            temp_time = TrafficTimer[GREEN_IDX];
            break;
        case MAN_AMBER:
            temp_time = TrafficTimer[AMBER_IDX];
            break;
        default:
            break;
    }
}

void fsm_manual_run() {
    // 1. Xử lý nháy LED (dùng Timer)
    if (timer_flag[TIMER_BLINK] == 1) {
        setTimer(TIMER_BLINK, 500);
        switch (STATUS) {
            case MAN_RED: blink_Red(); break;
            case MAN_GREEN: blink_Green(); break;
            case MAN_AMBER: blink_Amber(); break;
        }
    }

    // 2. Hiển thị 7SEG: (Mã Mode - Giá trị đang chỉnh)
    // Mode Red=2, Green=3, Amber=4
    set7SEGValues(STATUS, temp_time);

    // 3. Xử lý nút tăng thời gian
    if (isTimePress()) {
        temp_time++;
        if (temp_time > 99) temp_time = 1;
    }

    // 4. Xử lý nút SET (Lưu)
    if (isSetPress()) {
        // Ở đây ta lưu trực tiếp
        switch (STATUS) {
            case MAN_RED:
                TrafficTimer[RED_IDX] = temp_time;
                STATUS = MAN_GREEN;
                init_fsm_manual();
                break;
            case MAN_GREEN:
                TrafficTimer[GREEN_IDX] = temp_time;
                STATUS = MAN_AMBER;
                init_fsm_manual();
                break;
            case MAN_AMBER:
                TrafficTimer[AMBER_IDX] = temp_time;
                valid_time();
                STATUS = AUTO;
                init_fsm_auto();
                break;
        }
    }

    // 5. Xử lý nút MODE (Chuyển tiếp trạng thái)
    if (isModePress()) {
        switch (STATUS) {
            case MAN_RED:
                STATUS = MAN_GREEN;
                init_fsm_manual();
                break;
            case MAN_GREEN:
                STATUS = MAN_AMBER;
                init_fsm_manual();
                break;
            case MAN_AMBER:
                STATUS = AUTO;
                valid_time();
                init_fsm_auto(); // Quay về Auto
                break;
        }
    }
}
void valid_time() {
	if ((TrafficTimer[RED_IDX]
			< TrafficTimer[GREEN_IDX] + TrafficTimer[AMBER_IDX])) {
		TrafficTimer[RED_IDX] = TrafficTimer[GREEN_IDX]
				+ TrafficTimer[AMBER_IDX];
	} else if (TrafficTimer[RED_IDX]
			> TrafficTimer[GREEN_IDX] + TrafficTimer[AMBER_IDX]) {
		TrafficTimer[GREEN_IDX] = ceil(0.7 * TrafficTimer[RED_IDX]);
		TrafficTimer[AMBER_IDX] = TrafficTimer[RED_IDX]
				- TrafficTimer[GREEN_IDX];
	}
}
