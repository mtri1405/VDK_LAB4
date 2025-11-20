/* timer.c */
#include "timer.h"

int timer_counter[NO_OF_TIMERS];
int timer_flag[NO_OF_TIMERS];

void setTimer(int index, int duration) {
    // duration là số ms. Chia cho chu kỳ để ra số tick
    timer_counter[index] = duration / TIMER_CYCLE;
    timer_flag[index] = 0;
}

void timerRun() {
    for (int i = 0; i < NO_OF_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            if (timer_counter[i] <= 0) {
                timer_flag[i] = 1;
            }
        }
    }
}
