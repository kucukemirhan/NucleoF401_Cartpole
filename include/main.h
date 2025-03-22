/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

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
#define ENC2_CH1_Pin GPIO_PIN_0
#define ENC2_CH1_GPIO_Port GPIOA
#define ENC2_CH2_Pin GPIO_PIN_1
#define ENC2_CH2_GPIO_Port GPIOA
#define ENC1_CH1_Pin GPIO_PIN_5
#define ENC1_CH1_GPIO_Port GPIOA
#define ReverseEnable_Pin GPIO_PIN_0
#define ReverseEnable_GPIO_Port GPIOB
#define ForwardEnable_Pin GPIO_PIN_1
#define ForwardEnable_GPIO_Port GPIOB
#define RPWM_Pin GPIO_PIN_8
#define RPWM_GPIO_Port GPIOA
#define ENC1_CH2_Pin GPIO_PIN_3
#define ENC1_CH2_GPIO_Port GPIOB
#define LPWM_Pin GPIO_PIN_6
#define LPWM_GPIO_Port GPIOB
#define InputCapture1_Pin GPIO_PIN_8
#define InputCapture1_GPIO_Port GPIOB
#define InputCapture2_Pin GPIO_PIN_9
#define InputCapture2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
