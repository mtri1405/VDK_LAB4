/* global.c */
#include "global.h"

int STATUS = INIT;
int TrafficTimer[3] = {5, 3, 2}; // Default: Red 5s, Green 3s, Amber 2s

void run() {
    switch (STATUS) {
        case INIT:
            // Khởi tạo trạng thái ban đầu
            STATUS = AUTO;
            init_fsm_auto();
            break;

        case AUTO:
            fsm_auto_run();
            break;

        case MAN_RED:
        case MAN_GREEN:
        case MAN_AMBER:
            fsm_manual_run();
            break;

        default:
            STATUS = INIT;
            break;
    }
}
