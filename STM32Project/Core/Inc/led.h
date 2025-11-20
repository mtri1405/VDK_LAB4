/* led.h */
#ifndef INC_LED_H_
#define INC_LED_H_

#include "global.h"

void init_traffic_lights();
void setTrafficGreenRed();
void setTrafficAmberRed();
void setTrafficRedGreen();
void setTrafficRedAmber();

// Hàm cho mode Manual
void blink_Red();
void blink_Green();
void blink_Amber();
void turn_off_all();

void SYS_LED_Blinky();
#endif /* INC_LED_H_ */
