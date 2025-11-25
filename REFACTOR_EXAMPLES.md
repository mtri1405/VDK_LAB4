# Practical Refactoring Examples

## EXAMPLE 1: LED Module Refactoring

### BEFORE (Current Code - Problems)

**led.h**
```c
void init_traffic_lights();
void setTrafficRedGreen();
void setTrafficRedAmber();
void setTrafficGreenRed();
void setTrafficAmberRed();
void blink_Red();
void blink_Green();
void blink_Amber();
void turn_off_all();
void SYS_LED_Blinky();
```

**Problems:**
- ❌ Inconsistent naming (snake_case, camelCase mixed)
- ❌ Function names too long and repetitive
- ❌ No clear intent (what does "RedGreen" mean? Lane A RED + Lane B GREEN?)
- ❌ Procedural approach - duplicated GPIO writes
- ❌ No error checking or return codes
- ❌ No documentation

### AFTER (Improved - Clean Architecture)

**led.h** (Improved)
```c
#ifndef INC_LED_H_
#define INC_LED_H_

#include <stdint.h>
#include "main.h"
#include "config.h"

/* ============ LED TRAFFIC STATES ============ */
typedef enum {
    LIGHT_RED,      /* Off phase */
    LIGHT_AMBER,    /* Caution phase */
    LIGHT_GREEN,    /* Go phase */
} TrafficLightState;

/* ============ LANE CONFIGURATION ============ */
typedef enum {
    LANE_A = 0,
    LANE_B = 1,
} LaneID;

/* ============ INITIALIZATION ============ */
/**
 * @brief Initialize LED system - turn all off
 */
void led_init(void);

/* ============ TRAFFIC LIGHT CONTROL ============ */
/**
 * @brief Set traffic light state for specific lane
 * 
 * @param lane   LANE_A or LANE_B
 * @param state  LIGHT_RED, LIGHT_AMBER, or LIGHT_GREEN
 * 
 * @return 0 on success, -1 on invalid parameter
 * 
 * @note This turns off all lights on this lane, then sets the new one
 */
int led_set_traffic_state(LaneID lane, TrafficLightState state);

/**
 * @brief Convenience function: Set lane A RED + lane B GREEN
 */
void led_set_traffic_red_green(void);

/**
 * @brief Convenience function: Set lane A RED + lane B AMBER
 */
void led_set_traffic_red_amber(void);

/**
 * @brief Convenience function: Set lane A GREEN + lane B RED
 */
void led_set_traffic_green_red(void);

/**
 * @brief Convenience function: Set lane A AMBER + lane B RED
 */
void led_set_traffic_amber_red(void);

/* ============ BLINK/FLASH CONTROL ============ */
/**
 * @brief Toggle traffic light for manual mode (manual mode blinking)
 * 
 * @param lane  LANE_A or LANE_B
 * @param state Which color to toggle (RED, GREEN, AMBER)
 * 
 * @return 0 on success, -1 on error
 */
int led_toggle_traffic_light(LaneID lane, TrafficLightState state);

/**
 * @brief Blink RED on both lanes (manual mode)
 */
void led_blink_red_lanes(void);

/**
 * @brief Blink GREEN on both lanes (manual mode)
 */
void led_blink_green_lanes(void);

/**
 * @brief Blink AMBER on both lanes (manual mode)
 */
void led_blink_amber_lanes(void);

/**
 * @brief Turn off all traffic lights
 */
void led_turn_off_all(void);

/* ============ HEARTBEAT/SYSTEM LED ============ */
/**
 * @brief Toggle system heartbeat LED (red LED on PA05)
 * Call every 100ms to blink at 1 Hz
 */
void led_blink_heartbeat(void);

#endif /* INC_LED_H_ */
```

**led.c** (Improved)
```c
#include "led.h"

/* ============ GPIO PORT MAPPING ============ */
#define GPIO_PORT_A GPIOA
#define GPIO_PORT_B GPIOB

typedef struct {
    uint16_t red_pin;
    uint16_t green_pin;
    uint16_t amber_pin;
} LaneGPIOConfig;

static LaneGPIOConfig lanes[2] = {
    {LED_A_RED_Pin, LED_A_GREEN_Pin, LED_A_AMBER_Pin},     /* LANE_A */
    {LED_B_RED_Pin, LED_B_GREEN_Pin, LED_B_AMBER_Pin},     /* LANE_B */
};

/* Helper: Get pin for specific light */
static uint16_t get_light_pin(LaneID lane, TrafficLightState state) {
    if (lane >= 2) return 0;
    
    switch (state) {
        case LIGHT_RED:   return lanes[lane].red_pin;
        case LIGHT_GREEN: return lanes[lane].green_pin;
        case LIGHT_AMBER: return lanes[lane].amber_pin;
        default:          return 0;
    }
}

/* Helper: Get GPIO port for lane */
static GPIO_TypeDef* get_gpio_port(LaneID lane) {
    return (lane == LANE_A) ? GPIO_PORT_A : GPIO_PORT_B;
}

/* ============ INITIALIZATION ============ */
void led_init(void) {
    led_turn_off_all();
}

/* ============ TRAFFIC LIGHT CONTROL ============ */
int led_set_traffic_state(LaneID lane, TrafficLightState state) {
    GPIO_TypeDef* port = get_gpio_port(lane);
    uint16_t pin = get_light_pin(lane, state);
    
    if (pin == 0) return -1;  /* Invalid parameters */
    
    /* Turn off all lights on this lane first */
    HAL_GPIO_WritePin(port, lanes[lane].red_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, lanes[lane].green_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(port, lanes[lane].amber_pin, GPIO_PIN_RESET);
    
    /* Turn on the requested light */
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    
    return 0;
}

void led_set_traffic_red_green(void) {
    led_set_traffic_state(LANE_A, LIGHT_RED);
    led_set_traffic_state(LANE_B, LIGHT_GREEN);
}

void led_set_traffic_red_amber(void) {
    led_set_traffic_state(LANE_A, LIGHT_RED);
    led_set_traffic_state(LANE_B, LIGHT_AMBER);
}

void led_set_traffic_green_red(void) {
    led_set_traffic_state(LANE_A, LIGHT_GREEN);
    led_set_traffic_state(LANE_B, LIGHT_RED);
}

void led_set_traffic_amber_red(void) {
    led_set_traffic_state(LANE_A, LIGHT_AMBER);
    led_set_traffic_state(LANE_B, LIGHT_RED);
}

/* ============ BLINK/FLASH CONTROL ============ */
int led_toggle_traffic_light(LaneID lane, TrafficLightState state) {
    GPIO_TypeDef* port = get_gpio_port(lane);
    uint16_t pin = get_light_pin(lane, state);
    
    if (pin == 0) return -1;
    
    HAL_GPIO_TogglePin(port, pin);
    return 0;
}

void led_blink_red_lanes(void) {
    led_toggle_traffic_light(LANE_A, LIGHT_RED);
    led_toggle_traffic_light(LANE_B, LIGHT_RED);
}

void led_blink_green_lanes(void) {
    led_toggle_traffic_light(LANE_A, LIGHT_GREEN);
    led_toggle_traffic_light(LANE_B, LIGHT_GREEN);
}

void led_blink_amber_lanes(void) {
    led_toggle_traffic_light(LANE_A, LIGHT_AMBER);
    led_toggle_traffic_light(LANE_B, LIGHT_AMBER);
}

void led_turn_off_all(void) {
    for (int lane = 0; lane < 2; lane++) {
        GPIO_TypeDef* port = get_gpio_port(lane);
        HAL_GPIO_WritePin(port, lanes[lane].red_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(port, lanes[lane].green_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(port, lanes[lane].amber_pin, GPIO_PIN_RESET);
    }
}

void led_blink_heartbeat(void) {
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}
```

### KEY IMPROVEMENTS:
✅ **Enum-based states** - no magic numbers  
✅ **Lookup tables** - eliminates code duplication  
✅ **Clear naming** - consistent snake_case  
✅ **Helper functions** - DRY principle  
✅ **Documentation** - every function has purpose  
✅ **Return codes** - error handling  
✅ **No globals** - all config in structs  

---

## EXAMPLE 2: Button Module Refactoring

### BEFORE
```c
int KeyReg0[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int KeyReg1[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int KeyReg2[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int KeyReg3[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int TimeOutForKeyPress[NO_OF_BUTTONS] = {0, 0, 0};
int button_flag[NO_OF_BUTTONS] = {0, 0, 0};

void getKeyInput() {
    for (int i = 0; i < NO_OF_BUTTONS; i++) {
        KeyReg0[i] = KeyReg1[i];
        KeyReg1[i] = KeyReg2[i];
        KeyReg2[i] = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN[i]);
        
        if ((KeyReg0[i] == KeyReg1[i]) && (KeyReg1[i] == KeyReg2[i])) {
            if (KeyReg3[i] != KeyReg2[i]) {
                KeyReg3[i] = KeyReg2[i];
                if (KeyReg2[i] == PRESS_STATE) {
                    button_flag[i] = 1;        // ❌ Race condition
                    TimeOutForKeyPress[i] = DURATION_FOR_AUTO_INCREASING;
                }
            } else {
                TimeOutForKeyPress[i]--;
                if (TimeOutForKeyPress[i] == 0) {
                    if (KeyReg2[i] == PRESS_STATE) {
                        // button_flag[i] = 1;
                    }
                    TimeOutForKeyPress[i] = DURATION_FOR_AUTO_INCREASING;
                }
            }
        }
    }
}

int isButtonPress(int index) {
    if (button_flag[index] == 1) {
        button_flag[index] = 0;  // ❌ Global state mutation
        return 1;
    }
    return 0;
}
```

**Problems:**
- ❌ Unclear variable names (KeyReg0, KeyReg1 - register purposes unclear)
- ❌ Race condition: flag set in one function, read in another
- ❌ Global state pollution
- ❌ No clear debouncing documentation
- ❌ Timeout logic coupled with debounce logic

### AFTER (Improved)

**button.h** (Improved)
```c
#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>
#include "main.h"
#include "config.h"

/* Button indices for clarity */
typedef enum {
    BTN_MODE = 0,
    BTN_TIME = 1,
    BTN_SET  = 2,
} ButtonID;

/* Button events that callers can react to */
typedef enum {
    BTN_EVENT_PRESS,           /* Press edge (down) */
    BTN_EVENT_RELEASE,         /* Release edge (up) */
    BTN_EVENT_LONG_PRESS,      /* Held > 2 seconds */
} ButtonEvent;

/**
 * @brief Initialize button subsystem
 * Call once at startup
 */
void button_init(void);

/**
 * @brief Scan buttons for state changes
 * Must be called every TIMER_TICK_MS (10ms) from scheduler
 * 
 * @details Implements hardware debouncing using 4-register filter
 *          and edge detection for reliable press/release signals
 */
void button_scan(void);

/**
 * @brief Check if button was just pressed (edge detection)
 * 
 * @param btn_id Button ID (BTN_MODE, BTN_TIME, or BTN_SET)
 * 
 * @return 1 if pressed, 0 otherwise
 * 
 * @note Returns 1 only ONCE per press sequence
 *       Call repeatedly until next press to get single event
 */
uint8_t button_is_pressed(ButtonID btn_id);

/**
 * @brief Check if button is currently held down
 * 
 * @return 1 if button physically held, 0 if released
 */
uint8_t button_is_held(ButtonID btn_id);

/**
 * @brief Check if button was held > 2 seconds
 * 
 * @return 1 if long press detected, 0 otherwise
 * 
 * @note Also returns 1 once per long press sequence
 */
uint8_t button_is_long_pressed(ButtonID btn_id);

/* Convenience functions */
uint8_t button_mode_pressed(void);
uint8_t button_time_pressed(void);
uint8_t button_set_pressed(void);

#endif /* INC_BUTTON_H_ */
```

**button.c** (Improved)
```c
#include "button.h"

/* ============ BUTTON STATE MACHINE ============ */
typedef enum {
    STATE_IDLE,           /* Button released, waiting for press */
    STATE_DEBOUNCING,     /* Waiting for 3 stable reads */
    STATE_PRESSED,        /* Press confirmed, event fired once */
    STATE_HELD,           /* Button held, monitoring time */
    STATE_LONG_PRESSED,   /* Held > 2 seconds, event fired once */
} ButtonStateInternal;

typedef struct {
    /* Debounce registers (4-stage filter) */
    uint8_t read[4];
    
    /* Debounce & state tracking */
    ButtonStateInternal state;
    uint16_t hold_timer;        /* Milliseconds held */
    
    /* Edge detection flags (one-shot) */
    uint8_t press_edge_fired;   /* Prevent multiple fires */
    uint8_t long_press_fired;
    
} ButtonContext;

/* Button hardware mapping */
#define BUTTON_GPIO_PORT GPIOA
static uint16_t button_pins[3] = {MODE_Pin, TIME_Pin, SET_Pin};

/* Button contexts - one per physical button */
static ButtonContext buttons[3] = {0};

/* ============ HELPER: Check button GPIO ============ */
static uint8_t read_button_gpio(ButtonID btn_id) {
    if (btn_id >= 3) return GPIO_BTN_RELEASED;
    return HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, button_pins[btn_id]);
}

/* ============ INITIALIZATION ============ */
void button_init(void) {
    for (int i = 0; i < 3; i++) {
        buttons[i].read[0] = GPIO_BTN_RELEASED;
        buttons[i].read[1] = GPIO_BTN_RELEASED;
        buttons[i].read[2] = GPIO_BTN_RELEASED;
        buttons[i].read[3] = GPIO_BTN_RELEASED;
        buttons[i].state = STATE_IDLE;
        buttons[i].hold_timer = 0;
        buttons[i].press_edge_fired = 0;
        buttons[i].long_press_fired = 0;
    }
}

/* ============ MAIN DEBOUNCE & STATE MACHINE ============ */
void button_scan(void) {
    for (ButtonID btn_id = 0; btn_id < 3; btn_id++) {
        ButtonContext* btn = &buttons[btn_id];
        
        /* --- STAGE 1: Read GPIO into debounce register --- */
        btn->read[0] = btn->read[1];
        btn->read[1] = btn->read[2];
        btn->read[2] = btn->read[3];
        btn->read[3] = read_button_gpio(btn_id);
        
        /* --- STAGE 2: Debounce check (all 4 must match) --- */
        uint8_t stable_value = btn->read[0];
        uint8_t is_stable = (btn->read[0] == btn->read[1]) &&
                           (btn->read[1] == btn->read[2]) &&
                           (btn->read[2] == btn->read[3]);
        
        if (!is_stable) {
            continue;  /* Not ready yet, try next button */
        }
        
        /* --- STAGE 3: State machine --- */
        switch (btn->state) {
            case STATE_IDLE:
                /* Waiting for press */
                if (stable_value == GPIO_BTN_PRESSED) {
                    btn->state = STATE_PRESSED;
                    btn->press_edge_fired = 0;
                    btn->hold_timer = 0;
                    btn->long_press_fired = 0;
                }
                break;
                
            case STATE_PRESSED:
                /* Press just detected, fire edge event once */
                if (!btn->press_edge_fired) {
                    btn->press_edge_fired = 1;
                }
                
                /* Transition to held state */
                btn->state = STATE_HELD;
                btn->hold_timer = 0;
                break;
                
            case STATE_HELD:
                /* Monitor how long button stays pressed */
                btn->hold_timer += TIMER_TICK_MS;
                
                if (stable_value == GPIO_BTN_RELEASED) {
                    /* Button released */
                    btn->state = STATE_IDLE;
                    btn->hold_timer = 0;
                } else if (btn->hold_timer >= BUTTON_HOLD_TIME_MS) {
                    /* Long press threshold reached */
                    if (!btn->long_press_fired) {
                        btn->long_press_fired = 1;
                    }
                    btn->state = STATE_LONG_PRESSED;
                }
                break;
                
            case STATE_LONG_PRESSED:
                /* Already in long press, wait for release */
                if (stable_value == GPIO_BTN_RELEASED) {
                    btn->state = STATE_IDLE;
                    btn->hold_timer = 0;
                }
                break;
        }
    }
}

/* ============ PUBLIC API ============ */
uint8_t button_is_pressed(ButtonID btn_id) {
    if (btn_id >= 3) return 0;
    
    ButtonContext* btn = &buttons[btn_id];
    
    /* Return 1 only once per press sequence */
    if (btn->press_edge_fired) {
        btn->press_edge_fired = 0;  /* Consume event */
        return 1;
    }
    return 0;
}

uint8_t button_is_held(ButtonID btn_id) {
    if (btn_id >= 3) return 0;
    return (buttons[btn_id].state >= STATE_PRESSED);
}

uint8_t button_is_long_pressed(ButtonID btn_id) {
    if (btn_id >= 3) return 0;
    
    ButtonContext* btn = &buttons[btn_id];
    
    if (btn->long_press_fired) {
        btn->long_press_fired = 0;  /* Consume event */
        return 1;
    }
    return 0;
}

/* Convenience functions */
uint8_t button_mode_pressed(void) {
    return button_is_pressed(BTN_MODE);
}

uint8_t button_time_pressed(void) {
    return button_is_pressed(BTN_TIME);
}

uint8_t button_set_pressed(void) {
    return button_is_pressed(BTN_SET);
}
```

### KEY IMPROVEMENTS:
✅ **Explicit state machine** - clear button lifecycle  
✅ **One-shot event firing** - no missed or duplicate events  
✅ **Debounce algorithm documented** - 4-register filter with stability check  
✅ **Encapsulated state** - not exposed as globals  
✅ **Edge detection** - press_edge_fired flag prevents race  
✅ **Long press support** - with timeout handling  
✅ **Type-safe** - enum for ButtonID instead of magic ints  

---

## EXAMPLE 3: Timer Module Bug Fix

### BEFORE (Buggy)
```c
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
```

**Problem:** After decrement, `timer_counter[i]` can become -1, -2, -3... indefinitely
- It never stays at 0
- Next cycle: condition `timer_counter[i] > 0` is false, so it doesn't tick
- This is actually OK by accident, but semantically wrong

### AFTER (Fixed)
```c
void timerRun(void) {
    for (int i = 0; i < NO_OF_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            
            /* Set flag when counter reaches exactly 0 */
            if (timer_counter[i] == 0) {
                timer_flag[i] = 1;
            }
        }
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

**Improvements:**
✅ **Explicit `== 0`** - not `<= 0`  
✅ **Helper functions** - public API instead of direct flag access  
✅ **Bounds checking** - prevent buffer overflow  

---

## EXAMPLE 4: FSM Decoupling with Events

### BEFORE (Tight Coupling)
```c
// fsm_auto.c
if (isModePress()) {
    STATUS = MAN_RED;
    init_fsm_manual();  // ❌ Direct call to other module
}

// fsm_manual.c
void init_fsm_manual() {
    turn_off_all();  // ❌ Calls led.c directly
    // ...
}
```

**Problem:** fsm_auto.c knows about fsm_manual.c internals
- Changes to manual mode break auto mode
- No clear initialization order
- Hard to test each FSM independently

### AFTER (Event-Driven)
```c
// fsm_auto.c - just publish event
if (button_mode_pressed()) {
    system_set_mode(SYS_STATUS_MAN_RED);
    event_publish(EVENT_TRANSITION_TO_MANUAL_RED, NULL);
}

// fsm_manual.c - subscribe to event
static void on_enter_manual_red(EventType evt, void* data) {
    fsm_manual_init(SYS_STATUS_MAN_RED);
}

// main.c - wire them together
int main(void) {
    event_subscribe(EVENT_TRANSITION_TO_MANUAL_RED, on_enter_manual_red);
    // ...
}
```

**Benefits:**
✅ **Loose coupling** - modules don't know about each other  
✅ **Centralized wiring** - easy to see dependencies  
✅ **Easy to test** - mock event system  
✅ **Extensible** - add new subscribers without changing publishers  

---

## EXAMPLE 5: Configuration Centralization

### BEFORE
```c
// button.c
#define DURATION_FOR_AUTO_INCREASING 200  // What units? What value?

// timer.c
#define TIMER_CYCLE 10  // Unclear what this is

// scheduler.c
#define SCH_MAX_TASKS 20  // Duplicated if in multiple files

// 7_SEGMENT.c
setTimer(TIMER_TRAFFIC, 1000);  // 1000 what? ms? ticks?
```

**Problems:**
- ❌ Magic numbers scattered everywhere
- ❌ No single source of truth
- ❌ Hard to change timing without checking all files
- ❌ Units ambiguous

### AFTER (config.h)
```c
/* config.h */

/* ============ TIMING ============ */
#define TIMER_TICK_MS               10      // Scheduler tick = 10ms
#define TIMER_TRAFFIC_CYCLE_MS      1000    // Traffic countdown every 1 second
#define TIMER_BLINK_CYCLE_MS        500     // Blink cycle 0.5 seconds

#define BUTTON_HOLD_TIME_MS         2000    // Long press = 2 seconds
#define BUTTON_DEBOUNCE_MS          (4 * TIMER_TICK_MS)  // 40ms total

/* ============ SCHEDULER ============ */
#define SCH_MAX_TASKS               20

/* ============ TRAFFIC TIMINGS ============ */
#define DEFAULT_RED_TIME_SEC        5
#define DEFAULT_GREEN_TIME_SEC      3
#define DEFAULT_AMBER_TIME_SEC      2
```

**Usage in code:**
```c
// timer.c
setTimer(TIMER_TRAFFIC, TIMER_TRAFFIC_CYCLE_MS);  // Clear!

// button.c
if (hold_timer_ms >= BUTTON_HOLD_TIME_MS) {  // Named constant

// main.c
SCH_Add_Task(timerRun, 0, 1);  // Period = 1 tick = TIMER_TICK_MS ms
```

**Benefits:**
✅ **Single source of truth** - change once, affects whole system  
✅ **Self-documenting** - constant names explain values  
✅ **Easy retuning** - compile with different config  
✅ **Type-safe** - no magic numbers in code  

---

## MIGRATION CHECKLIST

- [ ] Create config.h with all #defines
- [ ] Create system_state.h/c to encapsulate globals
- [ ] Update all .c files to include "config.h"
- [ ] Replace all magic numbers with named constants
- [ ] Update all function names to snake_case
- [ ] Add parameter documentation to all functions
- [ ] Replace global variable access with getter/setter functions
- [ ] Create event_system.h/c
- [ ] Replace cross-module function calls with events
- [ ] Fix timer underflow bug
- [ ] Fix scheduler task deletion race
- [ ] Improve button debouncing
- [ ] Test each module independently
- [ ] Test state transitions
- [ ] Run static analysis (cppcheck)

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-24
