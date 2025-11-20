/* led.c */
#include "led.h"

void init_traffic_lights() {
    turn_off_all();
}

void turn_off_all() {
    HAL_GPIO_WritePin(LED_A_RED_GPIO_Port, LED_A_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_A_GREEN_GPIO_Port, LED_A_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_A_AMBER_GPIO_Port, LED_A_AMBER_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LED_B_RED_GPIO_Port, LED_B_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_B_GREEN_GPIO_Port, LED_B_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_B_AMBER_GPIO_Port, LED_B_AMBER_Pin, GPIO_PIN_RESET);
}

// Logic Auto
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
void setTrafficGreenRed() {
    turn_off_all();
    HAL_GPIO_WritePin(LED_A_GREEN_GPIO_Port, LED_A_GREEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_RED_GPIO_Port, LED_B_RED_Pin, GPIO_PIN_SET);
}
void setTrafficAmberRed() {
    turn_off_all();
    HAL_GPIO_WritePin(LED_A_AMBER_GPIO_Port, LED_A_AMBER_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B_RED_GPIO_Port, LED_B_RED_Pin, GPIO_PIN_SET);
}

// Logic Manual (Blink)
void blink_Red() {
    HAL_GPIO_TogglePin(LED_A_RED_GPIO_Port, LED_A_RED_Pin);
    HAL_GPIO_TogglePin(LED_B_RED_GPIO_Port, LED_B_RED_Pin);
}
void blink_Green() {
    HAL_GPIO_TogglePin(LED_A_GREEN_GPIO_Port, LED_A_GREEN_Pin);
    HAL_GPIO_TogglePin(LED_B_GREEN_GPIO_Port, LED_B_GREEN_Pin);
}
void blink_Amber() {
    HAL_GPIO_TogglePin(LED_A_AMBER_GPIO_Port, LED_A_AMBER_Pin);
    HAL_GPIO_TogglePin(LED_B_AMBER_GPIO_Port, LED_B_AMBER_Pin);
}

void SYS_LED_Blinky(){
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}
