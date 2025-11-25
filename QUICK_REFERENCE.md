# Quick Reference Guide - Code Improvements

## 1. FILE ORGANIZATION (New Structure)

```
STM32Project/Core/
├── Inc/
│   ├── main.h                      (STM32 generated)
│   ├── config.h              ✨ NEW - All constants
│   ├── system_state.h        ✨ NEW - Global state container
│   ├── event_system.h        ✨ NEW - Pub-Sub pattern
│   ├── scheduler.h           (IMPROVED - better docs)
│   ├── timer.h               (IMPROVED - fixed underflow)
│   ├── button.h              (IMPROVED - debounce fixed)
│   ├── led.h                 (IMPROVED - cleaner API)
│   ├── display_7seg.h        (REFACTORED)
│   ├── fsm_auto.h            (IMPROVED - less coupled)
│   ├── fsm_manual.h          (IMPROVED - less coupled)
│   └── [other HAL files]
│
└── Src/
    ├── main.c                (IMPROVED - orchestration)
    ├── config.c              (if needed for const arrays)
    ├── system_state.c        ✨ NEW
    ├── event_system.c        ✨ NEW
    ├── scheduler.c           (FIXED bugs)
    ├── timer.c               (FIXED bugs)
    ├── button.c              (IMPROVED)
    ├── led.c                 (IMPROVED)
    ├── display_7seg.c        (REFACTORED)
    ├── fsm_auto.c            (REFACTORED)
    ├── fsm_manual.c          (REFACTORED)
    └── [other source files]
```

---

## 2. TOP 10 MOST IMPORTANT CHANGES

| # | Issue | Old Code | New Code | Why |
|---|-------|----------|----------|-----|
| 1 | **Naming** | `setTrafficRedGreen()` | `led_set_traffic_red_green()` | Clear, consistent |
| 2 | **Globals** | `int STATUS;` | `system_get_mode()` | Encapsulated, traceable |
| 3 | **Magic Numbers** | `200, 1000, 9` | `BUTTON_HOLD_TIME_MS`, `TIMER_TICK_MS` | Self-documenting |
| 4 | **Timer Bug** | `timer_counter[i] <= 0` | `timer_counter[i] == 0` | Prevents underflow |
| 5 | **Task Deletion** | Delete during dispatch | Collect first, execute | No race condition |
| 6 | **Button Race** | Set/clear flag both places | Clear only on read | Single event |
| 7 | **Coupling** | `fsm_auto.c` calls `fsm_manual.c` | Events published | Decoupled modules |
| 8 | **Documentation** | Missing comments | Doxygen format | Maintainable |
| 9 | **Error Handling** | None | Return codes | Robust |
| 10 | **State** | Multiple globals | `SystemState` struct | Centralized |

---

## 3. NAMING CONVENTION QUICK GUIDE

### Variable Names
```c
// ❌ BEFORE (inconsistent)
int Status, STATUS, current_status, currentStatus;
uint8_t is_active, IsActive, isActive;

// ✅ AFTER (consistent snake_case)
int system_status;
uint8_t is_active;
```

### Function Names
```c
// ❌ BEFORE (mixed case)
void setTrafficRedGreen();        // camelCase
void init_traffic_lights();       // snake_case
void SYS_LED_Blinky();            // UPPER_SNAKE

// ✅ AFTER (all snake_case)
void led_set_traffic_red_green();
void led_init();
void led_blink_heartbeat();
```

### File Names
```c
// ❌ BEFORE
7_SEGMENT.c              // Numbers at start
stm32f1xx_it.c           // OK (HAL generated)

// ✅ AFTER
display_7seg.c           // Descriptive name
stm32f1xx_it.c           (unchanged)
```

### Constants
```c
// ❌ BEFORE (unclear units/purpose)
#define DURATION_FOR_AUTO_INCREASING 200
#define TIMER_CYCLE 10

// ✅ AFTER (clear and self-documenting)
#define BUTTON_HOLD_TIME_MS 2000
#define TIMER_TICK_MS 10
#define DEFAULT_RED_TIME_SEC 5
```

---

## 4. BUG FIXES SUMMARY

### Bug #1: Timer Underflow
```c
// ❌ OLD (goes negative indefinitely)
timer_counter[i]--;
if (timer_counter[i] <= 0) {
    timer_flag[i] = 1;
}
// After first decrement: -1, -2, -3...

// ✅ NEW (stops at 0)
if (--timer_counter[i] <= 0) {
    timer_counter[i] = 0;
    timer_flag[i] = 1;
}
// Or better:
if (timer_counter[i] > 0) {
    timer_counter[i]--;
    if (timer_counter[i] == 0) {
        timer_flag[i] = 1;
    }
}
```

### Bug #2: Task Deletion During Dispatch
```c
// ❌ OLD (modifies array while iterating)
void SCH_Dispatch_Tasks(void) {
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (SCH_tasks_G[i].RunMe > 0) {
            (*SCH_tasks_G[i].pTask)();
            if (SCH_tasks_G[i].Period == 0) {
                SCH_Delete_Task(i);  // ❌ Modifies array!
            }
        }
    }
}

// ✅ NEW (collect first, then execute)
void SCH_Dispatch_Tasks(void) {
    uint8_t tasks_to_run[SCH_MAX_TASKS];
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (SCH_tasks_G[i].RunMe > 0 && SCH_tasks_G[i].pTask != NULL) {
            tasks_to_run[count++] = i;
        }
    }
    
    for (uint8_t i = 0; i < count; i++) {
        uint8_t idx = tasks_to_run[i];
        if (SCH_tasks_G[idx].pTask != NULL && SCH_tasks_G[idx].RunMe > 0) {
            (*SCH_tasks_G[idx].pTask)();
            SCH_tasks_G[idx].RunMe--;
            
            if (SCH_tasks_G[idx].Period == 0) {
                SCH_Delete_Task(idx);
            }
        }
    }
}
```

### Bug #3: Button Flag Race Condition
```c
// ❌ OLD (flag reset in read function - race condition)
int button_flag[NO_OF_BUTTONS] = {0, 0, 0};

int isButtonPress(int index) {
    if (button_flag[index] == 1) {
        button_flag[index] = 0;  // Cleared here
        return 1;
    }
    return 0;
}

void getKeyInput() {
    // ... debounce logic ...
    if (KeyReg2[i] == PRESS_STATE) {
        button_flag[i] = 1;      // Set here
    }
}
// Problem: If getKeyInput runs, then FSM calls isButtonPress twice,
// first call gets 1, second call gets 0. Event might be missed.

// ✅ NEW (encapsulated state machine)
typedef struct {
    uint8_t read[4];
    uint8_t state;
    uint8_t press_edge_fired;   // One-shot flag
} ButtonContext;

static ButtonContext buttons[NO_OF_BUTTONS];

uint8_t button_is_pressed(ButtonID btn_id) {
    if (buttons[btn_id].press_edge_fired) {
        buttons[btn_id].press_edge_fired = 0;  // Consume
        return 1;
    }
    return 0;
}

void button_scan(void) {
    // ... debounce detects press edge ...
    if (/* press detected */) {
        buttons[i].press_edge_fired = 1;
    }
}
// Advantage: Clear state machine, no race condition
```

---

## 5. COUPLING REDUCTION

### Pattern 1: Direct Call (BAD)
```c
// fsm_auto.c
if (isModePress()) {
    init_fsm_manual();  // ❌ Knows about other module
}
```

**Problems:**
- Hard to test fsm_auto independently
- Changes to fsm_manual break fsm_auto
- Circular dependencies possible

### Pattern 2: Event-Driven (GOOD)
```c
// config.h
typedef enum {
    EVENT_TRANSITION_TO_MANUAL_RED,
    EVENT_TRANSITION_TO_MANUAL_GREEN,
    // ...
} EventType;

// fsm_auto.c
if (button_mode_pressed()) {
    system_set_mode(SYS_STATUS_MAN_RED);
    event_publish(EVENT_TRANSITION_TO_MANUAL_RED, NULL);
    // ✅ No longer knows about fsm_manual
}

// fsm_manual.c (or main.c)
void on_enter_manual_red(EventType evt, void* data) {
    fsm_manual_init(SYS_STATUS_MAN_RED);
}

event_subscribe(EVENT_TRANSITION_TO_MANUAL_RED, on_enter_manual_red);
```

**Advantages:**
- ✅ fsm_auto doesn't know about fsm_manual
- ✅ Easy to test with event system mock
- ✅ Easy to add observers later
- ✅ Clear dependencies

---

## 6. STATE MANAGEMENT PATTERN

### Old Way (Scattered Globals)
```c
// global.h
int STATUS;
int TrafficTimer[3];

// button.c
int button_flag[NO_OF_BUTTONS];

// timer.c
int timer_counter[NO_OF_TIMERS];

// fsm_auto.c
int status_auto;
int timeLeft_Lane1, timeLeft_Lane2;

// fsm_manual.c
int temp_time;

// Problem: State scattered everywhere, hard to understand
```

### New Way (Centralized State)
```c
// system_state.h
typedef struct {
    uint8_t  current_mode;
    uint8_t  previous_mode;
    int      traffic_timer_sec[3];
    int      time_left_lane1;
    int      time_left_lane2;
    int      temp_time_editing;
    uint8_t  mode_changed;
} SystemState;

extern SystemState sys_state;

// Accessors prevent direct modification
uint8_t system_get_mode(void);
void system_set_mode(uint8_t mode);
int system_get_time_left_lane1(void);
void system_set_time_left(int lane1, int lane2);

// system_state.c
SystemState sys_state = {0};

void system_set_mode(uint8_t mode) {
    sys_state.previous_mode = sys_state.current_mode;
    sys_state.current_mode = mode;
    sys_state.mode_changed = 1;  // ✅ Tracking change
}

// Advantages:
// ✅ All state in one place
// ✅ Controlled access through functions
// ✅ Can add validation in setters
// ✅ Easy to debug and trace
```

---

## 7. BEFORE/AFTER COMPARISON

### Module: LED Control

**BEFORE (Mixed, Repetitive)**
```c
// led.c - lots of duplication
void setTrafficRedGreen() {
    turn_off_all();
    HAL_GPIO_WritePin(LED_A_RED_GPIO_Port, LED_A_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_GREEN_GPIO_Port, LED_B_GREEN_Pin, GPIO_PIN_SET);
}

void setTrafficRedAmber() {
    turn_off_all();
    HAL_GPIO_WritePin(LED_A_RED_GPIO_Port, LED_A_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_AMBER_GPIO_Port, LED_B_AMBER_Pin, GPIO_PIN_SET);
}

void blink_Red() {
    HAL_GPIO_TogglePin(LED_A_RED_GPIO_Port, LED_A_RED_Pin);
    HAL_GPIO_TogglePin(LED_B_RED_GPIO_Port, LED_B_RED_Pin);
}
```

**AFTER (Clean, DRY)**
```c
// led.c - table-driven, no repetition
typedef struct {
    uint16_t red_pin;
    uint16_t green_pin;
    uint16_t amber_pin;
} LaneGPIOConfig;

static LaneGPIOConfig lanes[2] = {
    {LED_A_RED_Pin, LED_A_GREEN_Pin, LED_A_AMBER_Pin},
    {LED_B_RED_Pin, LED_B_GREEN_Pin, LED_B_AMBER_Pin},
};

int led_set_traffic_state(LaneID lane, TrafficLightState state) {
    // ✅ Reusable function, no duplication
    // ✅ Error checking
    // ✅ Clear parameters
}

void led_set_traffic_red_green(void) {
    led_set_traffic_state(LANE_A, LIGHT_RED);
    led_set_traffic_state(LANE_B, LIGHT_GREEN);
}
```

---

## 8. DOCUMENTATION TEMPLATE

### Function Documentation (Doxygen Format)
```c
/**
 * @brief   Short description of what function does
 * 
 * @details Longer explanation:
 *          - Algorithm details
 *          - Performance characteristics
 *          - Special cases
 *
 * @param   param1  [Type] Description with valid range
 * @param   param2  [Type] Description with units
 *
 * @return  [Type] Return value meaning
 *          - 0:  Success
 *          - -1: Error (specify which)
 *          - 1:  Timeout occurred
 *
 * @note    Important usage notes or side effects
 *
 * @warning Potential issues or pitfalls
 *
 * @see     related_function()
 *
 * @example
 *     @code
 *     if (button_is_pressed(BTN_MODE)) {
 *         system_set_mode(SYS_STATUS_MAN_RED);
 *     }
 *     @endcode
 */
uint8_t button_is_pressed(ButtonID btn_id);
```

---

## 9. TESTING CHECKLIST

```
CRITICAL BUGS (Must Fix)
[ ] Timer doesn't underflow
[ ] Task deletion doesn't crash
[ ] Button events not missed
[ ] Mode transitions complete

FUNCTIONALITY
[ ] Auto mode cycles through states
[ ] Manual mode allows time adjustment
[ ] Time display updates
[ ] LED blinks correctly

EDGE CASES
[ ] Min time (1 second) works
[ ] Max time (99 seconds) works
[ ] Rapid mode switching handled
[ ] Button held > 2 seconds
[ ] Quick button presses detected

PERFORMANCE
[ ] No tasks dropped
[ ] Display updates smooth (no lag)
[ ] Scheduler handles 20 tasks
[ ] No memory corruption

ROBUSTNESS
[ ] State valid after power cycle
[ ] Timeout values enforced
[ ] Invalid states handled
[ ] GPIO pins don't get stuck
```

---

## 10. IMPLEMENTATION PHASES

### Phase 1: Foundation (2-3 hours)
- [ ] Create config.h
- [ ] Create system_state.h/c
- [ ] Create event_system.h/c
- [ ] Update scheduler.c (task deletion fix)
- [ ] Update timer.c (underflow fix)

### Phase 2: Refactoring (3-4 hours)
- [ ] Update all function names (snake_case)
- [ ] Update button.c (debouncing)
- [ ] Update led.c (table-driven)
- [ ] Update fsm_*.c (event-driven)
- [ ] Update main.c (orchestration)

### Phase 3: Documentation (1-2 hours)
- [ ] Add Doxygen comments to all functions
- [ ] Document config.h constants
- [ ] Add usage examples

### Phase 4: Testing (2-3 hours)
- [ ] Unit test each module
- [ ] Integration test state transitions
- [ ] Test edge cases
- [ ] Hardware test on board

**Total Effort:** ~8-12 hours

---

## 11. COMMON MISTAKES TO AVOID

| Mistake | Wrong | Correct |
|---------|-------|---------|
| **Naming** | `setTraffic()` | `led_set_traffic_red_green()` |
| **Magic numbers** | `setTimer(idx, 1000)` | `setTimer(idx, TIMER_TRAFFIC_MS)` |
| **No validation** | `system_set_time(x)` | Check bounds in setter |
| **Direct access** | `sys_state.mode = 5` | `system_set_mode(SYS_STATUS_AUTO)` |
| **Global includes** | Every file includes global.h | Only include needed files |
| **Tight coupling** | FSM A calls FSM B | Publish event, let B subscribe |
| **No docs** | Function exists but unclear | Doxygen comment with example |
| **Race conditions** | Check flag twice | State machine, one-shot event |
| **Memory bugs** | Array access w/o bounds | Always check index bounds |
| **Silent failures** | No error codes | Return status, check in caller |

---

## 12. RESOURCES FOR LEARNING

**C Best Practices:**
- CERT Secure Coding Standard
- MISRA C:2012 Guidelines
- Embedded C book by Michael Barr

**Design Patterns:**
- Observer Pattern (for events)
- State Pattern (for FSM)
- Singleton Pattern (for system state)

**Tools:**
- `cppcheck` - Static analysis
- `clang-format` - Auto-formatting
- `doxygen` - Doc generation
- `valgrind` - Memory checking

**STM32 Specific:**
- HAL API Documentation
- AN4667 - STM32Cube Ecosystem
- UM1850 - STM32 Programming Manual

---

**Quick Links in This Repo:**
- Full guide: `IMPROVEMENT_GUIDE.md`
- Code examples: `REFACTOR_EXAMPLES.md`
- This reference: `QUICK_REFERENCE.md`

**Document Version:** 1.0  
**Last Updated:** 2025-11-24  
**Status:** Ready to Use
