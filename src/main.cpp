/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "CartpoleLib.h"
#include "stm32_ros.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t uartPrintFlag = 0;
volatile uint8_t rosPrintFlag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void myCustomCallback(TIM_HandleTypeDef *htim)
{
  uartPrintFlag = 0;
  rosPrintFlag = 1;
}

void ftoa(float value, char *buffer, int precision) {
  int int_part = (int)value; // Tam sayı kısmı
  int dec_part = (int)((value - int_part) * pow(10, precision)); // Ondalık kısmı

  sprintf(buffer, "%d.%0*d", int_part, precision, abs(dec_part)); // Stringe çevirme
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();

  MX_TIM5_Init();
  MX_USART1_UART_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */
  TimIT taskTimer(TIM9, &htim9);
  htim9.PeriodElapsedCallback = myCustomCallback;
  taskTimer.start();

  EncoderIT encoder1(&htim2);
  EncoderIT encoder2(&htim5);
  encoder1.start();
  encoder2.start();

  TimIC encCaptureTimer1(TIM10, &htim10);
  TimIC encCaptureTimer2(TIM11, &htim11);
  encCaptureTimer1.start();
  encCaptureTimer2.start();

  TimPWM ReversePWM(TIM1, &htim1);
  TimPWM ForwardPWM(TIM3, &htim3);
  ReversePWM.setFrequency(10000);
  ForwardPWM.setFrequency(5000);

  UartIT uart1(USART1, &huart1);
  uint8_t data[64] = {0};
  char uart_buffer[64] = {0};
  memset(uart_buffer, 0, sizeof(uart_buffer));
  
  uart1.start_read();

  int64_t encoder1_count = 0;
  int64_t encoder2_count = 0;
  
  float encoder1_speed = 0;
  float encoder2_speed = 0;
  char speed1_str[16];  
  char speed2_str[16];

  int64_t encoder1_count_ROS = 0;
  int64_t encoder2_count_ROS = 0;
  float encoder1_speed_ROS = 0;
  float encoder2_speed_ROS = 0;

  DCmotor cart_motor(encoder2, ForwardPWM, ReversePWM, encCaptureTimer2);
  // /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    // aldığın verilere göre değişkenleri set et
    cart_motor.setTargetSpeed(encoder1_speed_ROS); // 50 rpm

    // güncel değerleri oku
    encoder1_count = encoder1.read() % 4096;
    if (encoder1_count < 0)
    {
      encoder1_count += 4096;
    }
    
    encoder2_count = encoder2.read();

    encoder1_speed = encCaptureTimer1.getSpeed();
    encoder2_speed = encCaptureTimer2.getSpeed();

    uint16_t len = uart1.read((uint8_t*)uart_buffer, sizeof(uart_buffer) - 1);
    if (len > 0) {
        uart_buffer[len] = '\0';
        char *p = (char*)uart_buffer;

        // Yalnızca başında 's' varsa ilerle ve çevir
        if (*p == 's' || *p == 'S') {
            p++;
            encoder1_speed_ROS = strtof(p, nullptr);
        }
        // başında 's' yoksa buraya girmeden geçer
    }
    
    cart_motor.updateSpeed();

  
    // HABERLEŞME KISMI . . . . . . . . . . . . . . . . . . . . . . . . . . 
    if (rosPrintFlag && uart1.is_tx_complete())
    {
        rosPrintFlag = 0;
        ftoa(encoder1_speed, speed1_str, 2);
        ftoa(encoder2_speed, speed2_str, 2);
        
        int len = snprintf(uart_buffer, sizeof(uart_buffer),
            "%ld,%s,%ld,%s\n", 
            (long int)encoder2_count,
            speed2_str,
            (long int)encoder1_count, 
            speed1_str
        );
        if (len > 0)
        {
            uart1.write((uint8_t*)uart_buffer, len);
        }
    }
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
