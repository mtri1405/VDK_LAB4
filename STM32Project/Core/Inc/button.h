/* button.h */
#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "main.h"

#define NORMAL_STATE GPIO_PIN_SET
#define PRESS_STATE  GPIO_PIN_RESET

// Định nghĩa các nút
#define BTN_MODE 0
#define BTN_TIME 1
#define BTN_SET  2

void getKeyInput();
int isButtonPress(int index);
// Helper functions
int isModePress();
int isTimePress();
int isSetPress();

#endif /* INC_BUTTON_H_ */
