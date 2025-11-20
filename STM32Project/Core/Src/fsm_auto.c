/* fsm_auto.c */
#include "fsm_auto.h"

#define S_RED_GREEN 0
#define S_RED_AMBER 1
#define S_GREEN_RED 2
#define S_AMBER_RED 3

int status_auto = S_RED_GREEN;
int timeLeft_Lane1 = 0;
int timeLeft_Lane2 = 0;

void init_fsm_auto() {
    status_auto = S_RED_GREEN;
    timeLeft_Lane1 = TrafficTimer[RED_IDX];
    timeLeft_Lane2 = TrafficTimer[GREEN_IDX];
    setTrafficRedGreen();
    setTimer(TIMER_TRAFFIC, 1000); // 1 giây đếm lùi
}

void fsm_auto_run() {
    // 1. Xử lý đếm ngược thời gian
    if (timer_flag[TIMER_TRAFFIC] == 1) {
        setTimer(TIMER_TRAFFIC, 1000); // Reset timer 1s
        timeLeft_Lane1--;
        timeLeft_Lane2--;
    }

    // 2. Hiển thị lên 7SEG
    set7SEGValues(timeLeft_Lane1, timeLeft_Lane2);

    // 3. Logic chuyển trạng thái
    switch (status_auto) {
        case S_RED_GREEN:
            if (timeLeft_Lane2 <= 0) { // Green lane 2 hết
                status_auto = S_RED_AMBER;
                timeLeft_Lane2 = TrafficTimer[AMBER_IDX];
                setTrafficRedAmber();
            }
            break;

        case S_RED_AMBER:
            if (timeLeft_Lane2 <= 0) { // Amber lane 2 hết -> Cả 2 chuyển
                status_auto = S_GREEN_RED;
                timeLeft_Lane1 = TrafficTimer[GREEN_IDX];
                timeLeft_Lane2 = TrafficTimer[RED_IDX];
                setTrafficGreenRed();
            }
            break;

        case S_GREEN_RED:
            if (timeLeft_Lane1 <= 0) { // Green lane 1 hết
                status_auto = S_AMBER_RED;
                timeLeft_Lane1 = TrafficTimer[AMBER_IDX];
                setTrafficAmberRed();
            }
            break;

        case S_AMBER_RED:
            if (timeLeft_Lane1 <= 0) { // Amber lane 1 hết -> Về đầu
                status_auto = S_RED_GREEN;
                timeLeft_Lane1 = TrafficTimer[RED_IDX];
                timeLeft_Lane2 = TrafficTimer[GREEN_IDX];
                setTrafficRedGreen();
            }
            break;
    }

    // 4. Kiểm tra nút nhấn để chuyển sang Manual
    if (isModePress()) {
        STATUS = MAN_RED;
        init_fsm_manual(); // Khởi tạo manual mode cho RED
    }
}
