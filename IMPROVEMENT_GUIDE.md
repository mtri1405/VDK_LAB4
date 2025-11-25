# Code Improvement Guide - Traffic Light Control System

## 1. ARCHITECTURE OVERVIEW & PROBLEMS

### Current Problem: Tight Coupling via god header
```
global.h
├── scheduler.h
├── button.h
├── led.h
├── fsm_auto.h
├── fsm_manual.h
└── timer.h

Issue: global.h is a "god header" - everything includes it, creating circular dependencies
```

### Improved Architecture: Layered & Modular
```
┌─────────────────────────────────────────────┐
│          MAIN (Scheduler Loop)              │
└────────────────┬────────────────────────────┘
                 │
    ┌────────────┼────────────┐
    │            │            │
┌───▼───┐   ┌───▼────┐   ┌──▼────┐
│ FSM   │   │ HAL    │   │ Timer │
│ Layer │   │ Drivers│   │ Base  │
└───┬───┘   └───┬────┘   └──┬────┘
    │           │           │
    └───────────┼───────────┘
                │
        ┌───────▼────────┐
        │  config.h      │
        │  (Constants)   │
        └────────────────┘
```

---

## 2. STEP 1: CREATE CONFIGURATION LAYER

### Why: Centralize all magic numbers and constants

**File: Core/Inc/config.h** (NEW)
```c
#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

/* ============ TIMING CONFIGURATION ============ */
#define TIMER_TICK_MS               10      // Scheduler tick period in milliseconds
#define TIMER_TRAFFIC_CYCLE_MS      1000    // Traffic light countdown cycle (1 second)
#define TIMER_BLINK_CYCLE_MS        500     // Manual mode blink cycle (0.5 second)
#define BUTTON_DEBOUNCE_CYCLES      3       // Debounce cycles (3 * 10ms = 30ms)
#define BUTTON_HOLD_TIME_MS         2000    // Long press threshold (2 seconds)

/* ============ SCHEDULER CONFIGURATION ============ */
#define SCH_MAX_TASKS               20      // Maximum number of concurrent tasks
#define TASK_RETURN_NORMAL          0
#define TASK_RETURN_ERROR           1

/* ============ TIMER MODULE CONFIGURATION ============ */
#define NO_OF_TIMERS                3       // Number of independent timers
#define TIMER_TRAFFIC               0       // Timer index for traffic countdown
#define TIMER_BLINK                 1       // Timer index for LED blink
#define TIMER_DEBUG                 2       // Timer index for debugging

/* ============ BUTTON CONFIGURATION ============ */
#define NO_OF_BUTTONS               3
#define BTN_MODE_IDX                0
#define BTN_TIME_IDX                1
#define BTN_SET_IDX                 2

/* ============ LED & TRAFFIC STATE ============ */
#define RED_IDX                     0
#define GREEN_IDX                   1
#define AMBER_IDX                   2

/* ============ SYSTEM MODES (STATUS) ============ */
#define SYS_STATUS_INIT             0
#define SYS_STATUS_AUTO             1
#define SYS_STATUS_MAN_RED          2
#define SYS_STATUS_MAN_GREEN        3
#define SYS_STATUS_MAN_AMBER        4

/* ============ GPIO ACTIVE LEVELS ============ */
#define GPIO_LED_ON                 GPIO_PIN_SET    // Active high
#define GPIO_LED_OFF                GPIO_PIN_RESET
#define GPIO_BTN_PRESSED            GPIO_PIN_RESET  // Active low (pull-up)
#define GPIO_BTN_RELEASED           GPIO_PIN_SET

/* ============ VALIDATION CONSTRAINTS ============ */
#define MIN_LIGHT_TIME_SEC          1
#define MAX_LIGHT_TIME_SEC          99
#define DEFAULT_RED_TIME_SEC        5
#define DEFAULT_GREEN_TIME_SEC      3
#define DEFAULT_AMBER_TIME_SEC      2
#define GREEN_RATIO                 0.7f    // Green = 70% of Red cycle

#endif /* INC_CONFIG_H_ */
```

---

## 3. STEP 2: CREATE STATE MANAGEMENT LAYER

### Why: Eliminate scattered global variables

**File: Core/Inc/system_state.h** (NEW)
```c
#ifndef INC_SYSTEM_STATE_H_
#define INC_SYSTEM_STATE_H_

#include <stdint.h>
#include "config.h"

/* System-wide state container */
typedef struct {
    uint8_t  current_mode;              // SYS_STATUS_*
    uint8_t  previous_mode;
    
    int      traffic_timer_sec[3];      // [RED, GREEN, AMBER] in seconds
    
    /* Runtime counters */
    int      time_left_lane1;           // For auto mode
    int      time_left_lane2;           // For auto mode
    int      temp_time_editing;         // For manual mode
    
    /* State flags */
    uint8_t  mode_changed;              // Set when mode transitions
    uint8_t  time_updated;              // Set when user updates time
    
} SystemState;

/* Global instance - accessed via getter/setter */
extern SystemState sys_state;

/* Initialization */
void system_state_init(void);

/* Getters */
uint8_t system_get_mode(void);
int* system_get_traffic_timers(void);
int system_get_time_left_lane1(void);
int system_get_time_left_lane2(void);
int system_get_editing_time(void);

/* Setters */
void system_set_mode(uint8_t mode);
void system_set_traffic_timer(uint8_t color_idx, int seconds);
void system_set_time_left(int lane1, int lane2);
void system_set_editing_time(int time_sec);
void system_mark_mode_changed(void);
void system_mark_time_updated(void);

/* Validators */
uint8_t system_validate_traffic_times(void);
void system_apply_default_times(void);

#endif /* INC_SYSTEM_STATE_H_ */
```

**File: Core/Src/system_state.c** (NEW)
```c
#include "system_state.h"

/* Global state instance */
SystemState sys_state = {0};

void system_state_init(void) {
    sys_state.current_mode = SYS_STATUS_INIT;
    sys_state.previous_mode = SYS_STATUS_INIT;
    sys_state.traffic_timer_sec[RED_IDX] = DEFAULT_RED_TIME_SEC;
    sys_state.traffic_timer_sec[GREEN_IDX] = DEFAULT_GREEN_TIME_SEC;
    sys_state.traffic_timer_sec[AMBER_IDX] = DEFAULT_AMBER_TIME_SEC;
    sys_state.time_left_lane1 = 0;
    sys_state.time_left_lane2 = 0;
    sys_state.temp_time_editing = DEFAULT_RED_TIME_SEC;
    sys_state.mode_changed = 0;
    sys_state.time_updated = 0;
}

uint8_t system_get_mode(void) {
    return sys_state.current_mode;
}

int* system_get_traffic_timers(void) {
    return sys_state.traffic_timer_sec;
}

int system_get_time_left_lane1(void) {
    return sys_state.time_left_lane1;
}

int system_get_time_left_lane2(void) {
    return sys_state.time_left_lane2;
}

int system_get_editing_time(void) {
    return sys_state.temp_time_editing;
}

void system_set_mode(uint8_t mode) {
    sys_state.previous_mode = sys_state.current_mode;
    sys_state.current_mode = mode;
    sys_state.mode_changed = 1;
}

void system_set_traffic_timer(uint8_t color_idx, int seconds) {
    if (color_idx >= 3) return;
    
    if (seconds < MIN_LIGHT_TIME_SEC)
        seconds = MIN_LIGHT_TIME_SEC;
    if (seconds > MAX_LIGHT_TIME_SEC)
        seconds = MAX_LIGHT_TIME_SEC;
    
    sys_state.traffic_timer_sec[color_idx] = seconds;
}

void system_set_time_left(int lane1, int lane2) {
    sys_state.time_left_lane1 = lane1;
    sys_state.time_left_lane2 = lane2;
}

void system_set_editing_time(int time_sec) {
    if (time_sec < MIN_LIGHT_TIME_SEC)
        time_sec = MIN_LIGHT_TIME_SEC;
    if (time_sec > MAX_LIGHT_TIME_SEC)
        time_sec = MAX_LIGHT_TIME_SEC;
    
    sys_state.temp_time_editing = time_sec;
    sys_state.time_updated = 1;
}

void system_mark_mode_changed(void) {
    sys_state.mode_changed = 1;
}

void system_mark_time_updated(void) {
    sys_state.time_updated = 1;
}

uint8_t system_validate_traffic_times(void) {
    int red = sys_state.traffic_timer_sec[RED_IDX];
    int green = sys_state.traffic_timer_sec[GREEN_IDX];
    int amber = sys_state.traffic_timer_sec[AMBER_IDX];
    
    /* RED must >= GREEN + AMBER */
    return (red >= (green + amber)) ? 1 : 0;
}

void system_apply_default_times(void) {
    system_state_init();
}
```

---

## 4. STEP 3: FIX CRITICAL BUGS

### Bug 1: Timer Underflow

**File: Core/Src/timer.c** - FIXED VERSION
```c
#include "timer.h"
#include "config.h"

int timer_counter[NO_OF_TIMERS] = {0};
int timer_flag[NO_OF_TIMERS] = {0};

void setTimer(int index, int duration_ms) {
    if (index >= NO_OF_TIMERS) return;
    
    /* Convert ms to scheduler ticks */
    timer_counter[index] = (duration_ms + TIMER_TICK_MS - 1) / TIMER_TICK_MS;
    timer_flag[index] = 0;
}

void timerRun(void) {
    for (int i = 0; i < NO_OF_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            
            if (timer_counter[i] == 0) {
                timer_flag[i] = 1;
            }
        }
        /* Else: timer is inactive, remain at 0 */
    }
}

int isTimerExpired(int index) {
    if (index >= NO_OF_TIMERS) return 0;
    return timer_flag[index];
}

void clearTimerFlag(int index) {
    if (index < NO_OF_TIMERS) {
        timer_flag[index] = 0;
    }
}
```

**File: Core/Inc/timer.h** - UPDATED
```c
#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include <stdint.h>
#include "config.h"

extern int timer_counter[NO_OF_TIMERS];
extern int timer_flag[NO_OF_TIMERS];

/**
 * @brief Initialize timer with duration in milliseconds
 * @param index: Timer index (0 to NO_OF_TIMERS-1)
 * @param duration_ms: Duration in milliseconds
 */
void setTimer(int index, int duration_ms);

/**
 * @brief Called by scheduler every TIMER_TICK_MS
 * Decrements all active timers and sets flags when expired
 */
void timerRun(void);

/**
 * @brief Check if timer has expired
 * @return 1 if expired, 0 otherwise
 */
int isTimerExpired(int index);

/**
 * @brief Clear timer expiration flag (manual reset)
 */
void clearTimerFlag(int index);

#endif /* INC_TIMER_H_ */
```

### Bug 2: Task Deletion During Dispatch

**File: Core/Src/scheduler.c** - FIXED VERSION
```c
#include "scheduler.h"
#include "config.h"

sTask SCH_tasks_G[SCH_MAX_TASKS];
uint8_t ReadyQueue[SCH_MAX_TASKS];
uint8_t ReadyQueueSize = 0;

void SCH_Insert_Into_Queue(uint8_t taskIndex) {
    uint8_t i = 0;
    
    for (i = 0; i < ReadyQueueSize; i++) {
        if (SCH_tasks_G[taskIndex].Delay < SCH_tasks_G[ReadyQueue[i]].Delay) {
            SCH_tasks_G[ReadyQueue[i]].Delay -= SCH_tasks_G[taskIndex].Delay;
            break;
        }
        SCH_tasks_G[taskIndex].Delay -= SCH_tasks_G[ReadyQueue[i]].Delay;
    }
    
    for (uint8_t j = ReadyQueueSize; j > i; j--) {
        ReadyQueue[j] = ReadyQueue[j - 1];
    }
    
    ReadyQueue[i] = taskIndex;
    ReadyQueueSize++;
}

uint8_t SCH_Remove_From_Queue(void) {
    if (ReadyQueueSize == 0) {
        return SCH_MAX_TASKS;
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

void SCH_Init(void) {
    ReadyQueueSize = 0;
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask = NULL;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
    }
}

uint8_t SCH_Add_Task(void (*pFunction)(void), uint16_t delay_ticks, uint16_t period_ticks) {
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (!SCH_tasks_G[i].pTask) {
            SCH_tasks_G[i].pTask = pFunction;
            SCH_tasks_G[i].Delay = delay_ticks;
            SCH_tasks_G[i].Period = period_ticks;
            SCH_tasks_G[i].RunMe = 0;
            
            SCH_Insert_Into_Queue(i);
            return i;
        }
    }
    return SCH_MAX_TASKS;  /* No free slot */
}

void SCH_Delete_Task(uint8_t taskIndex) {
    if (taskIndex >= SCH_MAX_TASKS || SCH_tasks_G[taskIndex].pTask == NULL) {
        return;
    }
    
    /* Find task in queue */
    uint8_t i = 0;
    for (i = 0; i < ReadyQueueSize; i++) {
        if (ReadyQueue[i] == taskIndex) break;
    }
    
    /* Remove from queue if found */
    if (i < ReadyQueueSize) {
        if (i < ReadyQueueSize - 1) {
            SCH_tasks_G[ReadyQueue[i+1]].Delay += SCH_tasks_G[ReadyQueue[i]].Delay;
        }
        
        for (uint8_t j = i; j < ReadyQueueSize - 1; j++) {
            ReadyQueue[j] = ReadyQueue[j+1];
        }
        ReadyQueueSize--;
    }
    
    /* Clear task data */
    SCH_tasks_G[taskIndex].pTask = NULL;
    SCH_tasks_G[taskIndex].Delay = 0;
    SCH_tasks_G[taskIndex].Period = 0;
    SCH_tasks_G[taskIndex].RunMe = 0;
}

void SCH_Update(void) {
    if (ReadyQueueSize == 0) return;
    
    if (SCH_tasks_G[ReadyQueue[0]].Delay > 0) {
        SCH_tasks_G[ReadyQueue[0]].Delay--;
    }
    
    /* Handle all tasks with Delay == 0 */
    while (ReadyQueueSize > 0 && SCH_tasks_G[ReadyQueue[0]].Delay == 0) {
        uint8_t taskIndex = ReadyQueue[0];
        SCH_tasks_G[taskIndex].RunMe += 1;
        SCH_Remove_From_Queue();
        
        if (SCH_tasks_G[taskIndex].Period > 0) {
            SCH_tasks_G[taskIndex].Delay = SCH_tasks_G[taskIndex].Period;
            SCH_Insert_Into_Queue(taskIndex);
        }
    }
}

void SCH_Dispatch_Tasks(void) {
    /* FIXED: Collect ready tasks first, then dispatch */
    uint8_t tasks_to_run[SCH_MAX_TASKS];
    uint8_t count = 0;
    
    /* Scan for ready tasks */
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (SCH_tasks_G[i].RunMe > 0 && SCH_tasks_G[i].pTask != NULL) {
            tasks_to_run[count++] = i;
        }
    }
    
    /* Execute ready tasks */
    for (uint8_t i = 0; i < count; i++) {
        uint8_t task_idx = tasks_to_run[i];
        
        /* Double-check task still exists */
        if (SCH_tasks_G[task_idx].pTask != NULL && SCH_tasks_G[task_idx].RunMe > 0) {
            (*SCH_tasks_G[task_idx].pTask)();
            SCH_tasks_G[task_idx].RunMe--;
            
            /* Delete one-time tasks after execution */
            if (SCH_tasks_G[task_idx].Period == 0) {
                SCH_Delete_Task(task_idx);
            }
        }
    }
}

void SCH_Go_To_Sleep(void) {
    __WFI();
}
```

---

## 5. STEP 4: REFACTOR MODULES WITH EVENT CALLBACKS

### Pattern: Replace direct function calls with callbacks

**File: Core/Inc/event_system.h** (NEW)
```c
#ifndef INC_EVENT_SYSTEM_H_
#define INC_EVENT_SYSTEM_H_

#include <stdint.h>

/* Event types */
typedef enum {
    EVENT_MODE_CHANGED,
    EVENT_TIME_UPDATED,
    EVENT_TRANSITION_TO_AUTO,
    EVENT_TRANSITION_TO_MANUAL_RED,
    EVENT_TRANSITION_TO_MANUAL_GREEN,
    EVENT_TRANSITION_TO_MANUAL_AMBER,
} EventType;

/* Event callback function pointer */
typedef void (*EventCallback)(EventType event, void* data);

/* Register callback for event */
void event_subscribe(EventType event, EventCallback callback);

/* Trigger event */
void event_publish(EventType event, void* data);

#endif /* INC_EVENT_SYSTEM_H_ */
```

**File: Core/Src/event_system.c** (NEW)
```c
#include "event_system.h"
#include "config.h"

#define MAX_SUBSCRIBERS 5

typedef struct {
    EventCallback callbacks[MAX_SUBSCRIBERS];
    uint8_t count;
} EventSubscribers;

static EventSubscribers subscribers[6] = {0};  /* One for each event type */

void event_subscribe(EventType event, EventCallback callback) {
    if (event >= 6 || subscribers[event].count >= MAX_SUBSCRIBERS) {
        return;
    }
    subscribers[event].callbacks[subscribers[event].count++] = callback;
}

void event_publish(EventType event, void* data) {
    if (event >= 6) return;
    
    for (uint8_t i = 0; i < subscribers[event].count; i++) {
        if (subscribers[event].callbacks[i] != NULL) {
            subscribers[event].callbacks[i](event, data);
        }
    }
}
```

---

## 6. STEP 5: DECOUPLE FSM MODULES

### Before: Direct coupling
```c
// fsm_auto.c calls fsm_manual.c directly
if (isModePress()) {
    STATUS = MAN_RED;
    init_fsm_manual();  // ❌ Tight coupling
}
```

### After: Event-driven decoupling
```c
// fsm_auto.c - publish event
if (isModePress()) {
    system_set_mode(SYS_STATUS_MAN_RED);
    event_publish(EVENT_TRANSITION_TO_MANUAL_RED, NULL);
}

// fsm_manual.c - subscribe to event
event_subscribe(EVENT_TRANSITION_TO_MANUAL_RED, on_enter_manual_red);
```

**File: Core/Inc/fsm_auto.h** (REFACTORED)
```c
#ifndef INC_FSM_AUTO_H_
#define INC_FSM_AUTO_H_

/**
 * @brief Initialize auto mode FSM
 * Called once when entering AUTO mode
 */
void fsm_auto_init(void);

/**
 * @brief Run auto mode logic
 * Called once per scheduler cycle in AUTO mode
 */
void fsm_auto_run(void);

#endif /* INC_FSM_AUTO_H_ */
```

**File: Core/Src/fsm_auto.c** (REFACTORED)
```c
#include "fsm_auto.h"
#include "system_state.h"
#include "config.h"
#include "led.h"
#include "button.h"
#include "timer.h"
#include "display_7seg.h"
#include "event_system.h"

/* Auto FSM states */
typedef enum {
    STATE_RED_GREEN,   /* Lane A: RED, Lane B: GREEN */
    STATE_RED_AMBER,   /* Lane A: RED, Lane B: AMBER */
    STATE_GREEN_RED,   /* Lane A: GREEN, Lane B: RED */
    STATE_AMBER_RED,   /* Lane A: AMBER, Lane B: RED */
} AutoState;

static AutoState current_state = STATE_RED_GREEN;

void fsm_auto_init(void) {
    current_state = STATE_RED_GREEN;
    int* timers = system_get_traffic_timers();
    
    system_set_time_left(timers[RED_IDX], timers[GREEN_IDX]);
    led_set_traffic_red_green();
    setTimer(TIMER_TRAFFIC, TIMER_TRAFFIC_CYCLE_MS);
}

void fsm_auto_run(void) {
    int* timers = system_get_traffic_timers();
    int time_lane1 = system_get_time_left_lane1();
    int time_lane2 = system_get_time_left_lane2();
    
    /* Countdown every second */
    if (isTimerExpired(TIMER_TRAFFIC)) {
        setTimer(TIMER_TRAFFIC, TIMER_TRAFFIC_CYCLE_MS);
        time_lane1--;
        time_lane2--;
        system_set_time_left(time_lane1, time_lane2);
    }
    
    /* Update display */
    display_set_values(time_lane1, time_lane2);
    
    /* State transitions */
    switch (current_state) {
        case STATE_RED_GREEN:
            if (time_lane2 <= 0) {
                current_state = STATE_RED_AMBER;
                time_lane2 = timers[AMBER_IDX];
                led_set_traffic_red_amber();
            }
            break;
            
        case STATE_RED_AMBER:
            if (time_lane2 <= 0) {
                current_state = STATE_GREEN_RED;
                time_lane1 = timers[GREEN_IDX];
                time_lane2 = timers[RED_IDX];
                led_set_traffic_green_red();
            }
            break;
            
        case STATE_GREEN_RED:
            if (time_lane1 <= 0) {
                current_state = STATE_AMBER_RED;
                time_lane1 = timers[AMBER_IDX];
                led_set_traffic_amber_red();
            }
            break;
            
        case STATE_AMBER_RED:
            if (time_lane1 <= 0) {
                current_state = STATE_RED_GREEN;
                time_lane1 = timers[RED_IDX];
                time_lane2 = timers[GREEN_IDX];
                led_set_traffic_red_green();
            }
            break;
    }
    
    system_set_time_left(time_lane1, time_lane2);
    
    /* Check mode switch */
    if (isModePress()) {
        system_set_mode(SYS_STATUS_MAN_RED);
        event_publish(EVENT_TRANSITION_TO_MANUAL_RED, NULL);
    }
}
```

---

## 7. STEP 6: IMPROVE BUTTON HANDLING (FIX RACE CONDITION)

**File: Core/Src/button.c** (REFACTORED)
```c
#include "button.h"
#include "config.h"

typedef struct {
    uint16_t reg[4];        /* 4-register debounce */
    uint16_t timeout;
    uint8_t is_pressed;     /* Current debounced state */
} ButtonState;

static ButtonState buttons[NO_OF_BUTTONS] = {0};

void button_init(void) {
    for (int i = 0; i < NO_OF_BUTTONS; i++) {
        buttons[i].reg[0] = GPIO_BTN_RELEASED;
        buttons[i].reg[1] = GPIO_BTN_RELEASED;
        buttons[i].reg[2] = GPIO_BTN_RELEASED;
        buttons[i].reg[3] = GPIO_BTN_RELEASED;
        buttons[i].is_pressed = 0;
        buttons[i].timeout = 0;
    }
}

void button_scan(void) {
    GPIO_TypeDef* port = GPIOA;
    uint16_t pins[NO_OF_BUTTONS] = {MODE_Pin, TIME_Pin, SET_Pin};
    
    for (int i = 0; i < NO_OF_BUTTONS; i++) {
        /* Shift registers */
        buttons[i].reg[0] = buttons[i].reg[1];
        buttons[i].reg[1] = buttons[i].reg[2];
        buttons[i].reg[2] = buttons[i].reg[3];
        buttons[i].reg[3] = HAL_GPIO_ReadPin(port, pins[i]);
        
        /* Debounce: all 4 reads must match */
        if ((buttons[i].reg[0] == buttons[i].reg[1]) &&
            (buttons[i].reg[1] == buttons[i].reg[2]) &&
            (buttons[i].reg[2] == buttons[i].reg[3])) {
            
            /* State changed */
            if (buttons[i].is_pressed != buttons[i].reg[3]) {
                buttons[i].is_pressed = buttons[i].reg[3];
                
                if (buttons[i].is_pressed == GPIO_BTN_PRESSED) {
                    buttons[i].timeout = BUTTON_HOLD_TIME_MS / TIMER_TICK_MS;
                }
            } else {
                /* Long press handling (optional) */
                if (buttons[i].timeout > 0) {
                    buttons[i].timeout--;
                }
            }
        }
    }
}

/**
 * @brief Check if button was pressed (edge detection)
 * Returns 1 only once per press, then 0 until next press
 */
uint8_t button_is_pressed(uint8_t btn_idx) {
    if (btn_idx >= NO_OF_BUTTONS) return 0;
    
    static uint8_t prev_state[NO_OF_BUTTONS] = {0};
    
    uint8_t current = buttons[btn_idx].is_pressed;
    uint8_t changed = (prev_state[btn_idx] != current);
    
    prev_state[btn_idx] = current;
    
    /* Return 1 only on press edge */
    return (changed && current == GPIO_BTN_PRESSED) ? 1 : 0;
}

uint8_t button_is_mode_pressed(void) {
    return button_is_pressed(BTN_MODE_IDX);
}

uint8_t button_is_time_pressed(void) {
    return button_is_pressed(BTN_TIME_IDX);
}

uint8_t button_is_set_pressed(void) {
    return button_is_pressed(BTN_SET_IDX);
}
```

---

## 8. STEP 7: IMPROVE MAIN.C (ORCHESTRATION)

**File: Core/Src/main.c** (REFACTORED - KEY PARTS)
```c
#include "main.h"
#include "config.h"
#include "system_state.h"
#include "scheduler.h"
#include "timer.h"
#include "button.h"
#include "led.h"
#include "display_7seg.h"
#include "fsm_auto.h"
#include "fsm_manual.h"
#include "event_system.h"

/* Forward declarations for callbacks */
static void on_transition_to_auto(EventType evt, void* data);
static void on_transition_to_manual_red(EventType evt, void* data);
static void on_transition_to_manual_green(EventType evt, void* data);
static void on_transition_to_manual_amber(EventType evt, void* data);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    
    /* Initialize layers */
    system_state_init();
    SCH_Init();
    button_init();
    led_init();
    display_init();
    
    /* Subscribe to state transition events */
    event_subscribe(EVENT_TRANSITION_TO_AUTO, on_transition_to_auto);
    event_subscribe(EVENT_TRANSITION_TO_MANUAL_RED, on_transition_to_manual_red);
    event_subscribe(EVENT_TRANSITION_TO_MANUAL_GREEN, on_transition_to_manual_green);
    event_subscribe(EVENT_TRANSITION_TO_MANUAL_AMBER, on_transition_to_manual_amber);
    
    /* Schedule tasks (all times in ticks) */
    SCH_Add_Task(timerRun, 0, 1);           /* Every 10ms */
    SCH_Add_Task(button_scan, 0, 1);        /* Every 10ms */
    SCH_Add_Task(system_dispatcher, 1, 1);  /* Every 10ms, slight delay */
    SCH_Add_Task(display_update, 2, 1);     /* Every 10ms */
    SCH_Add_Task(led_blink_heartbeat, 30, 100); /* Every 1 second */
    
    HAL_TIM_Base_Start_IT(&htim2);
    
    while (1) {
        SCH_Dispatch_Tasks();
    }
}

static void system_dispatcher(void) {
    uint8_t mode = system_get_mode();
    
    switch (mode) {
        case SYS_STATUS_INIT:
            system_set_mode(SYS_STATUS_AUTO);
            event_publish(EVENT_TRANSITION_TO_AUTO, NULL);
            break;
            
        case SYS_STATUS_AUTO:
            fsm_auto_run();
            break;
            
        case SYS_STATUS_MAN_RED:
        case SYS_STATUS_MAN_GREEN:
        case SYS_STATUS_MAN_AMBER:
            fsm_manual_run();
            break;
            
        default:
            system_set_mode(SYS_STATUS_INIT);
            break;
    }
}

static void on_transition_to_auto(EventType evt, void* data) {
    (void)evt;
    (void)data;
    fsm_auto_init();
}

static void on_transition_to_manual_red(EventType evt, void* data) {
    (void)evt;
    (void)data;
    fsm_manual_init(SYS_STATUS_MAN_RED);
}

static void on_transition_to_manual_green(EventType evt, void* data) {
    (void)evt;
    (void)data;
    fsm_manual_init(SYS_STATUS_MAN_GREEN);
}

static void on_transition_to_manual_amber(EventType evt, void* data) {
    (void)evt;
    (void)data;
    fsm_manual_init(SYS_STATUS_MAN_AMBER);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        SCH_Update();
    }
}
```

---

## 9. NAMING CONVENTION GUIDE

### Standard C Embedded Convention

| Category | Pattern | Example |
|----------|---------|---------|
| **Constants** | `UPPER_SNAKE_CASE` | `MAX_RETRIES`, `I2C_TIMEOUT_MS` |
| **Types** | `PascalCase` or `snake_case_t` | `SystemState` or `system_state_t` |
| **Functions** | `snake_case` | `button_is_pressed()`, `led_set_color()` |
| **Variables** | `snake_case` | `temp_count`, `is_active` |
| **Macros** | `UPPER_SNAKE_CASE` | `GPIO_LED_ON`, `TIMER_TICK_MS` |
| **Struct members** | `snake_case` | `struct.current_mode` |
| **File names** | `snake_case` | `system_state.h`, `fsm_auto.c` |
| **Pointers** | `*snake_case` or `p_snake_case` | `*buffer` or `p_data` |

### Before (Mixed)
```c
void setTrafficRedGreen();       // camelCase function
int TrafficTimer[3];              // PascalCase variable
#define NORMAL_STATE GPIO_PIN_SET // OK
int KeyReg0[NO_OF_BUTTONS];      // Mixed: Key camelCase, Reg Pascal
```

### After (Consistent)
```c
void led_set_traffic_red_green();        // snake_case function
int traffic_timer[3];                     // snake_case variable
#define STATE_NORMAL GPIO_PIN_SET        // UPPER_SNAKE_CASE macro
int button_register[NO_OF_BUTTONS];      // snake_case variable
```

---

## 10. DOCUMENTATION TEMPLATE

### Function Documentation
```c
/**
 * @brief   [One-line description of what function does]
 * 
 * @details [Detailed explanation if needed - algorithm, special cases, etc.]
 *
 * @param   param1   [Description and valid range]
 * @param   param2   [Description and valid range]
 *
 * @return  [Return value meaning]
 *          - 0: Success
 *          - 1: Parameter out of range
 *          - -1: Hardware error
 *
 * @note    [Important usage notes, side effects, etc.]
 *
 * @example
 *     @code
 *     if (button_is_pressed(BTN_MODE)) {
 *         // Handle mode button press
 *     }
 *     @endcode
 */
uint8_t button_is_pressed(uint8_t btn_idx);
```

### Module Documentation
```c
/**
 * @file    button.c
 * @brief   Button debouncing and state management
 *
 * @details This module provides:
 *          - Hardware debouncing (4-register filter)
 *          - Edge detection (press vs hold)
 *          - Timeout handling for long press
 *
 * @dependencies
 *          - config.h (timing constants)
 *          - main.h (GPIO definitions)
 *
 * @usage
 *     1. Call button_init() once at startup
 *     2. Call button_scan() every 10ms from scheduler
 *     3. Query button_is_pressed(BTN_IDX) in event handlers
 */
```

---

## 11. IMPLEMENTATION ROADMAP

### Phase 1: Foundation (Hour 1-2)
1. ✅ Create `config.h` - centralize constants
2. ✅ Create `system_state.h/c` - encapsulate global state
3. ✅ Create `event_system.h/c` - publish-subscribe pattern

### Phase 2: Bug Fixes (Hour 2-3)
4. ✅ Fix timer.c (underflow bug)
5. ✅ Fix scheduler.c (task deletion race)
6. ✅ Improve button.c (edge detection)

### Phase 3: Refactoring (Hour 3-5)
7. ✅ Update all .c files to use new naming conventions
8. ✅ Replace direct function calls with events
9. ✅ Add function documentation
10. ✅ Update main.c orchestration

### Phase 4: Testing (Hour 5+)
11. ✅ Test each module independently
12. ✅ Test state transitions
13. ✅ Test edge cases (min/max times)
14. ✅ Stress test scheduler (20 concurrent tasks)

---

## 12. HELPFUL RESOURCES

**Embedded C Best Practices:**
- MISRA C:2012 - Motor Industry Software Reliability Association
- CERT Secure Coding Standards
- Barr Group Embedded C Coding Standard

**Design Patterns:**
- State Machine Pattern (FSM)
- Observer Pattern (Event System)
- Singleton Pattern (System State)
- Factory Pattern (Task Creation)

**STM32 HAL:**
- STM32CubeMX Documentation
- HAL Driver API Reference
- Interrupt & Timer Best Practices

**Tools:**
- `cppcheck` - Static analysis
- `clang-format` - Code formatting
- `doxygen` - Documentation generation
- `valgrind` - Memory checking (simulation)

---

## 13. TESTING CHECKLIST

```
[ ] Timer doesn't go negative
[ ] Task dispatch doesn't skip tasks
[ ] Button press detected reliably
[ ] Mode transitions work (Auto -> ManRed -> ManGreen -> ManAmber -> Auto)
[ ] Time validation enforces constraints
[ ] 7-segment display updates smoothly
[ ] Scheduler handles 20 concurrent tasks
[ ] No memory leaks (all mallocs freed)
[ ] Edge cases: min/max timer values
[ ] Power consumption optimized (__WFI sleep)
```

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-24  
**Status:** Ready for Implementation
