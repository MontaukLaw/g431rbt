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
#include "stm32g4xx_hal.h"

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
#define ST_RTX_Pin GPIO_PIN_14
#define ST_RTX_GPIO_Port GPIOC
#define CHRG_Pin GPIO_PIN_15
#define CHRG_GPIO_Port GPIOC
#define D0_Pin GPIO_PIN_0
#define D0_GPIO_Port GPIOC
#define D1_Pin GPIO_PIN_1
#define D1_GPIO_Port GPIOC
#define D2_Pin GPIO_PIN_2
#define D2_GPIO_Port GPIOC
#define D3_Pin GPIO_PIN_3
#define D3_GPIO_Port GPIOC
#define D15_Pin GPIO_PIN_1
#define D15_GPIO_Port GPIOA
#define D14_Pin GPIO_PIN_2
#define D14_GPIO_Port GPIOA
#define D13_Pin GPIO_PIN_3
#define D13_GPIO_Port GPIOA
#define D12_Pin GPIO_PIN_4
#define D12_GPIO_Port GPIOA
#define D11_Pin GPIO_PIN_5
#define D11_GPIO_Port GPIOA
#define D10_Pin GPIO_PIN_6
#define D10_GPIO_Port GPIOA
#define D9_Pin GPIO_PIN_7
#define D9_GPIO_Port GPIOA
#define D8_Pin GPIO_PIN_4
#define D8_GPIO_Port GPIOC
#define D7_Pin GPIO_PIN_0
#define D7_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_1
#define D6_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_2
#define D5_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_10
#define D4_GPIO_Port GPIOB
#define FLASH_CS_Pin GPIO_PIN_12
#define FLASH_CS_GPIO_Port GPIOB
#define FLASH_CLK_Pin GPIO_PIN_13
#define FLASH_CLK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_14
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_15
#define FLASH_MOSI_GPIO_Port GPIOB
#define SW_4067_S3_Pin GPIO_PIN_6
#define SW_4067_S3_GPIO_Port GPIOC
#define SW_4067_S2_Pin GPIO_PIN_7
#define SW_4067_S2_GPIO_Port GPIOC
#define SW_4067_S1_Pin GPIO_PIN_8
#define SW_4067_S1_GPIO_Port GPIOC
#define SW_4067_S0_Pin GPIO_PIN_9
#define SW_4067_S0_GPIO_Port GPIOC
#define POWER_KEY_Pin GPIO_PIN_11
#define POWER_KEY_GPIO_Port GPIOA
#define LED_G_Pin GPIO_PIN_15
#define LED_G_GPIO_Port GPIOA
#define LED_R_Pin GPIO_PIN_10
#define LED_R_GPIO_Port GPIOC
#define LED_B_Pin GPIO_PIN_5
#define LED_B_GPIO_Port GPIOB
#define BL_LINK_Pin GPIO_PIN_7
#define BL_LINK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
