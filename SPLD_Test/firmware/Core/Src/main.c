/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "oled_sh1106.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  /* USER CODE BEGIN 2 */
  /* CPLD monitor inputs: PA3->IO28, PA11->IO33, PA12->IO34 (PA3 temp for IO28) */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin   = GPIO_PIN_3 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  OLED_Init();

  /* Layout: title, then 3 columns centered - label, circle, 0/1 */
  /* Column centers: 24, 64, 104. Circle radius 8, center y=28. Label y=10, digit y=40 */
  #define CPLD_CX0  24
  #define CPLD_CX1  64
  #define CPLD_CX2  104
  #define CPLD_CY   28
  #define CPLD_R    8
  #define LABEL_Y   10
  #define DIGIT_Y   40
  #define BOX_X0    4
  #define BOX_X1    44
  #define BOX_X2    84
  #define BOX_X3    124
  #define BOX_Y0    18
  #define BOX_Y1    52

  /* Static frame: border + title + labels */
  OLED_DrawRect(0, 0, OLED_WIDTH - 1, OLED_HEIGHT - 1);
  OLED_DrawString(36, 0, "CPLD MON");
  OLED_DrawString(14, LABEL_Y, "IO28");
  OLED_DrawString(54, LABEL_Y, "IO33");
  OLED_DrawString(94, LABEL_Y, "IO34");
  OLED_Refresh();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Read PA3, PA11, PA12 (CPLD IO28, IO33, IO34) */
    GPIO_PinState s0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
    GPIO_PinState s1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
    GPIO_PinState s2 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);

    /* Clear circle + digit area for each column */
    OLED_ClearRect(BOX_X0, BOX_Y0, BOX_X1, BOX_Y1);
    OLED_ClearRect(BOX_X1 + 2, BOX_Y0, BOX_X2, BOX_Y1);
    OLED_ClearRect(BOX_X2 + 2, BOX_Y0, BOX_X3, BOX_Y1);

    /* Redraw circles: LOW = hollow, HIGH = filled */
    OLED_DrawCircle(CPLD_CX0, CPLD_CY, CPLD_R, (s0 == GPIO_PIN_SET));
    OLED_DrawCircle(CPLD_CX1, CPLD_CY, CPLD_R, (s1 == GPIO_PIN_SET));
    OLED_DrawCircle(CPLD_CX2, CPLD_CY, CPLD_R, (s2 == GPIO_PIN_SET));

    /* 0 or 1 below each circle (centered: 6px font, so -3 from center) */
    OLED_DrawChar(CPLD_CX0 - 3, DIGIT_Y, (s0 == GPIO_PIN_SET) ? '1' : '0');
    OLED_DrawChar(CPLD_CX1 - 3, DIGIT_Y, (s1 == GPIO_PIN_SET) ? '1' : '0');
    OLED_DrawChar(CPLD_CX2 - 3, DIGIT_Y, (s2 == GPIO_PIN_SET) ? '1' : '0');

    OLED_Refresh();
    HAL_Delay(80);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
#ifdef USE_FULL_ASSERT
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
