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
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
const int MAX_LED = 4;
int index_led = 0;
volatile int led_buffer[4] = {0, 0, 0, 0};

int hour = 15;
int minute = 8;
int second = 50;

volatile int timer0_counter = 0;
volatile int timer0_flag = 0;

const int TIMER_CYCLE = 10; 
      /* TIM2 period: 10 ms */
volatile int timer1_counter = 0;
volatile int timer1_flag = 0;

volatile int timer2_counter = 0;
volatile int timer2_flag = 0;

const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;

/* Smaller A: 5 columns wide, 6 rows tall, with blank margins */
uint8_t matrix_buffer[8] = {
    0x00, 0x7C, 0x12, 0x12,
    0x12, 0x7C, 0x00, 0x00
};

uint32_t last_matrix_scan = 0;


uint32_t last_matrix_animation = 0;
int matrix_scroll_offset = 0;

/* Small A with margins, followed by eight blank columns */
const uint8_t scrolling_pattern[16] = {
    0x00, 0x7C, 0x12, 0x12,
    0x12, 0x7C, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void display7SEG(int num);
void update7SEG(int index);
void updateClockBuffer(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void scrollMatrixLeft(void) {
  matrix_scroll_offset++;

  if (matrix_scroll_offset >= 16) {
    matrix_scroll_offset = 0;
  }

  for (int col = 0; col < MAX_LED_MATRIX; col++) {
    int source = (col + matrix_scroll_offset) % 16;
    matrix_buffer[col] = scrolling_pattern[source];
  }
}

void updateLEDMatrix(int index) {
  const uint16_t all_columns = ENM0_Pin | ENM1_Pin | ENM2_Pin | ENM3_Pin |
                               ENM4_Pin | ENM5_Pin | ENM6_Pin | ENM7_Pin;

  uint16_t selected_column;

  if (index < 0 || index >= MAX_LED_MATRIX) {
    return;
  }

  /* Disable all columns before changing the row data. */
  HAL_GPIO_WritePin(GPIOA, all_columns, GPIO_PIN_SET);

  /* Update the eight active-low row outputs. */
  for (int row = 0; row < 8; row++) {
    uint16_t row_pin = (uint16_t)(GPIO_PIN_8 << row);

    GPIO_PinState row_state = (matrix_buffer[index] & (1U << row))
                                  ? GPIO_PIN_RESET /* Pixel ON */
                                  : GPIO_PIN_SET;  /* Pixel OFF */

    HAL_GPIO_WritePin(GPIOB, row_pin, row_state);
  }

  /* Select the column using the definitions in main.h. */
  switch (index) {
  case 0:
    selected_column = ENM0_Pin; /* PA2 */
    break;

  case 1:
    selected_column = ENM1_Pin; /* PA3 */
    break;

  case 2:
    selected_column = ENM2_Pin; /* PA10 */
    break;

  case 3:
    selected_column = ENM3_Pin; /* PA11 */
    break;

  case 4:
    selected_column = ENM4_Pin; /* PA12 */
    break;

  case 5:
    selected_column = ENM5_Pin; /* PA13 */
    break;

  case 6:
    selected_column = ENM6_Pin; /* PA14 */
    break;

  case 7:
    selected_column = ENM7_Pin; /* PA15 */
    break;

  default:
    return;
  }

  /* Enable only the selected column: active LOW. */
  HAL_GPIO_WritePin(GPIOA, selected_column, GPIO_PIN_RESET);
}

void setTimer0(int duration) {
  timer0_counter = duration / TIMER_CYCLE;
  timer0_flag = 0;
}

void setTimer1(int duration) {
  timer1_counter = duration / TIMER_CYCLE;
  timer1_flag = 0;
}

void setTimer2(int duration) {
  timer2_counter = duration / TIMER_CYCLE;
  timer2_flag = 0;
}

void timer_run(void) {
  if (timer0_counter > 0) {
    timer0_counter--;
    if (timer0_counter == 0) {
      timer0_flag = 1;
    }
  }

  if (timer1_counter > 0) {
    timer1_counter--;
    if (timer1_counter == 0) {
      timer1_flag = 1;
    }
  }

  if (timer2_counter > 0) {
    timer2_counter--;
    if (timer2_counter == 0) {
      timer2_flag = 1;
    }
  }
}

/* Put a number from 0 to 9 on the shared PB0-PB6 segment lines */
void display7SEG(int num){
    const uint16_t all_segments =
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
        GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;

    uint16_t segments_on = 0;

    switch (num)
    {
        case 0: /* a b c d e f */
            segments_on = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
            break;

        case 1: /* b c */
            segments_on = GPIO_PIN_1 | GPIO_PIN_2;
            break;

        case 2: /* a b d e g */
            segments_on = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_6;
            break;

        case 3: /* a b c d g */
            segments_on = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3 | GPIO_PIN_6;
            break;

        case 4: /* b c f g */
            segments_on = GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_5 | GPIO_PIN_6;
            break;

        case 5: /* a c d f g */
            segments_on = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_5 | GPIO_PIN_6;
            break;

        case 6: /* a c d e f g */
            segments_on = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
            break;

        case 7: /* a b c */
            segments_on = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
            break;

        case 8: /* a b c d e f g */
            segments_on = all_segments;
            break;

        case 9: /* a b c d f g */
            segments_on = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                          GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_6;
            break;

        default:
            break; /* Unsupported number: leave all segments off */
    }

    /* Common anode: HIGH = segment off, LOW = segment on */
    HAL_GPIO_WritePin(GPIOB, all_segments, GPIO_PIN_SET);

    if (segments_on != 0)
    {
        HAL_GPIO_WritePin(GPIOB, segments_on, GPIO_PIN_RESET);
    }
  }

/* Select display index 0, 1, 2, or 3 */
void update7SEG(int index){
    if (index < 0 || index >= MAX_LED)
    {
        return;
    }

    /* Disable every display before changing the shared segments */
    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
        GPIO_PIN_SET
    );

    switch (index)
    {
        case 0:
            display7SEG(led_buffer[0]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
            break;

        case 1:
            display7SEG(led_buffer[1]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
            break;

        case 2:
            display7SEG(led_buffer[2]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
            break;

        case 3:
            display7SEG(led_buffer[3]);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
            break;

        default:
            break;
    }
  }

void updateClockBuffer(void){
    led_buffer[0] = hour / 10;    /* Hour tens */
    led_buffer[1] = hour % 10;    /* Hour ones */
    led_buffer[2] = minute / 10;  /* Minute tens */
    led_buffer[3] = minute % 10;  /* Minute ones */
}

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
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_SET);

  updateClockBuffer();
  update7SEG(index_led); /* Show the first digit immediately */

  setTimer0(1000); /* PA5's first toggle */
  setTimer1(1000); /* Next clock second and DOT toggle */
  setTimer2(250);  /* Next display selection */

  HAL_TIM_Base_Start_IT(&htim2);

  updateLEDMatrix(index_led_matrix);
  last_matrix_scan = HAL_GetTick();

  
  last_matrix_animation = last_matrix_scan;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* Red LED: first toggle after 1 s, then every 2 s */
    if (timer0_flag == 1) {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
      setTimer0(2000);
    }

    /* Digital clock and DOT: update every 1 s */
    if (timer1_flag == 1) {
      setTimer1(1000);

      second++;

      if (second >= 60) {
        second = 0;
        minute++;
      }

      if (minute >= 60) {
        minute = 0;
        hour++;
      }

      if (hour >= 24) {
        hour = 0;
      }

      updateClockBuffer();
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
    }

    /* Seven-segment scan: change display every 250 ms */
    if (timer2_flag == 1) {
      setTimer2(250);

      index_led++;
      if (index_led >= MAX_LED) {
        index_led = 0;
      }

      update7SEG(index_led);
    }

    uint32_t now = HAL_GetTick();

    /* Scan one matrix column every 5 ms */
    if ((uint32_t)(now - last_matrix_scan) >= 5U) {
      last_matrix_scan = now;

      index_led_matrix++;
      if (index_led_matrix >= MAX_LED_MATRIX) {
        index_led_matrix = 0;
      }

      /* Move A at the beginning of a complete scan */
      if (index_led_matrix == 0 &&
          (uint32_t)(now - last_matrix_animation) >= 200U) {

        last_matrix_animation += 200U;
        scrollMatrixLeft();
      }

      updateLEDMatrix(index_led_matrix);
    }
    /* USER CODE END WHILE */

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
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG_D_Pin|SEG_E_Pin|SEG_F_Pin
                          |SEG_G_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : ENM0_Pin ENM1_Pin DOT_Pin LED_RED_Pin
                           EN0_Pin EN1_Pin EN2_Pin EN3_Pin
                           ENM2_Pin ENM3_Pin ENM4_Pin ENM5_Pin
                           ENM6_Pin ENM7_Pin */
  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG_A_Pin SEG_B_Pin SEG_C_Pin ROW2_Pin
                           ROW3_Pin ROW4_Pin ROW5_Pin ROW6_Pin
                           ROW7_Pin SEG_D_Pin SEG_E_Pin SEG_F_Pin
                           SEG_G_Pin ROW0_Pin ROW1_Pin */
  GPIO_InitStruct.Pin = SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG_D_Pin|SEG_E_Pin|SEG_F_Pin
                          |SEG_G_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer_run();
    }
}
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
