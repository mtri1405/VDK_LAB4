/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_A_RED_Pin GPIO_PIN_1
#define LED_A_RED_GPIO_Port GPIOA
#define LED_A_AMBER_Pin GPIO_PIN_2
#define LED_A_AMBER_GPIO_Port GPIOA
#define LED_A_GREEN_Pin GPIO_PIN_3
#define LED_A_GREEN_GPIO_Port GPIOA
#define EN_A_Pin GPIO_PIN_4
#define EN_A_GPIO_Port GPIOA
#define LED_RED_Pin GPIO_PIN_5
#define LED_RED_GPIO_Port GPIOA
#define LED_B_RED_Pin GPIO_PIN_6
#define LED_B_RED_GPIO_Port GPIOA
#define LED_B_AMBER_Pin GPIO_PIN_7
#define LED_B_AMBER_GPIO_Port GPIOA
#define Seg1_a_Pin GPIO_PIN_0
#define Seg1_a_GPIO_Port GPIOB
#define Seg1_b_Pin GPIO_PIN_1
#define Seg1_b_GPIO_Port GPIOB
#define Seg1_c_Pin GPIO_PIN_2
#define Seg1_c_GPIO_Port GPIOB
#define Seg2_d_Pin GPIO_PIN_10
#define Seg2_d_GPIO_Port GPIOB
#define Seg2_e_Pin GPIO_PIN_11
#define Seg2_e_GPIO_Port GPIOB
#define Seg2_f_Pin GPIO_PIN_12
#define Seg2_f_GPIO_Port GPIOB
#define Seg2_g_Pin GPIO_PIN_13
#define Seg2_g_GPIO_Port GPIOB
#define LED_B_GREEN_Pin GPIO_PIN_8
#define LED_B_GREEN_GPIO_Port GPIOA
#define EN_B_Pin GPIO_PIN_9
#define EN_B_GPIO_Port GPIOA
#define MODE_Pin GPIO_PIN_10
#define MODE_GPIO_Port GPIOA
#define TIME_Pin GPIO_PIN_11
#define TIME_GPIO_Port GPIOA
#define SET_Pin GPIO_PIN_12
#define SET_GPIO_Port GPIOA
#define Seg1_d_Pin GPIO_PIN_3
#define Seg1_d_GPIO_Port GPIOB
#define Seg1_e_Pin GPIO_PIN_4
#define Seg1_e_GPIO_Port GPIOB
#define Seg1_f_Pin GPIO_PIN_5
#define Seg1_f_GPIO_Port GPIOB
#define Seg1_g_Pin GPIO_PIN_6
#define Seg1_g_GPIO_Port GPIOB
#define Seg2_a_Pin GPIO_PIN_7
#define Seg2_a_GPIO_Port GPIOB
#define Seg2_b_Pin GPIO_PIN_8
#define Seg2_b_GPIO_Port GPIOB
#define Seg2_c_Pin GPIO_PIN_9
#define Seg2_c_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
