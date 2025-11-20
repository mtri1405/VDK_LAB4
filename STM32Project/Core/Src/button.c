/* button.c */
#include "button.h"

#define NO_OF_BUTTONS 3
#define DURATION_FOR_AUTO_INCREASING 200 // 2s hold

int KeyReg0[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int KeyReg1[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int KeyReg2[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
int KeyReg3[NO_OF_BUTTONS] = {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};

int TimeOutForKeyPress[NO_OF_BUTTONS] = {0, 0, 0};
int button_flag[NO_OF_BUTTONS] = {0, 0, 0};

// Mapping GPIO
GPIO_TypeDef* BUTTON_PORT = GPIOA;
uint16_t BUTTON_PIN[NO_OF_BUTTONS] = {MODE_Pin, TIME_Pin, SET_Pin};

int isButtonPress(int index) {
    if (button_flag[index] == 1) {
        button_flag[index] = 0;
        return 1;
    }
    return 0;
}

int isModePress() { return isButtonPress(BTN_MODE); }
int isTimePress() { return isButtonPress(BTN_TIME); }
int isSetPress()  { return isButtonPress(BTN_SET); }

void getKeyInput() {
    for (int i = 0; i < NO_OF_BUTTONS; i++) {
        KeyReg0[i] = KeyReg1[i];
        KeyReg1[i] = KeyReg2[i];
        KeyReg2[i] = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN[i]);

        if ((KeyReg0[i] == KeyReg1[i]) && (KeyReg1[i] == KeyReg2[i])) {
            if (KeyReg3[i] != KeyReg2[i]) {
                KeyReg3[i] = KeyReg2[i];
                if (KeyReg2[i] == PRESS_STATE) {
                    button_flag[i] = 1;
                    TimeOutForKeyPress[i] = DURATION_FOR_AUTO_INCREASING;
                }
            } else {
                TimeOutForKeyPress[i]--;
                if (TimeOutForKeyPress[i] == 0) {
                    if (KeyReg2[i] == PRESS_STATE) {
                        // Chế độ nhấn giữ (nếu cần)
                        // button_flag[i] = 1;
                    }
                    TimeOutForKeyPress[i] = DURATION_FOR_AUTO_INCREASING;
                }
            }
        }
    }
}
