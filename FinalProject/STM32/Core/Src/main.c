/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "ssd1306.h"
#include "si5351.h"
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
uint8_t tes;

uint32_t timerEnc = 0, timerOled = 0, timerDDS = 0;
uint32_t timerLed = 0;

int16_t rotEnc, countEnc = 0;

uint8_t i2c_val;

uint8_t Rx_buff[32];
uint8_t Rx_temp;
uint8_t Rx_state;

long freq_esp;

uint8_t button_biru, button_putih, button_kuning, button_merah;

uint8_t menu_active = 1;
uint8_t submenu_active = 0;
uint8_t subsubmenu_active = 0;

char *menu_items[] = {"Disable Clock", "Gen CLK ENC", "Gen CLK IOT"};
char *submenu_disable_items[] = {"Back"};
char *submenu_enc_items[] = {"Clock 0", "Clock 1", "Clock 2", "Back"};
char *submenu_iot_items[] = {"Back"};
char *units[] = {"Hz", "kHz", "MHz"};
int menu_index = 0;
int submenu_index = 0;
int unit_index = 0;
int16_t frequency = 0;
int active_submenu = 0; // 0: Disable, 1: ENC, 2: IOT
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void ScanningI2C() {
    HAL_StatusTypeDef res;
    for(uint16_t i = 0; i < 128; i++) {
        res = HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 10);
        if(res == HAL_OK) {
            i2c_val = 1;
        } else {
        	i2c_val = 0;
        }
    }
}

void init_dds() {
	ScanningI2C();
	const int32_t correction = 978;
	si5351_Init(correction);
	si5351_SetupCLK0(10000, SI5351_DRIVE_STRENGTH_4MA);
	si5351_SetupCLK1(20000, SI5351_DRIVE_STRENGTH_4MA);
	si5351_SetupCLK2(30000, SI5351_DRIVE_STRENGTH_4MA);
	si5351_EnableOutputs((1<<0) | (1<<1) | (1<<2));
}

void display_menu() {
    ssd1306_Fill(Black);
    for (int i = 0; i < 3; i++) {
        ssd1306_SetCursor(0, i * 10);
        ssd1306_WriteString(menu_items[i], Font_7x10, White);
        if (i == menu_index) {
            ssd1306_SetCursor(100, i * 10);
            ssd1306_WriteString("<--", Font_7x10, White);
        }
    }
    ssd1306_UpdateScreen(&hi2c2);
}

void display_submenu() {
    ssd1306_Fill(Black);
    char **submenu_items;
    int submenu_length;

    switch (active_submenu) {
        case 0:
            submenu_items = submenu_disable_items;
            submenu_length = 1;
            break;
        case 1:
            submenu_items = submenu_enc_items;
            submenu_length = 4;
            break;
        case 2:
            submenu_items = submenu_iot_items;
            submenu_length = 1;
            break;
    }

    for (int i = 0; i < submenu_length; i++) {
        ssd1306_SetCursor(0, i * 10);
        ssd1306_WriteString(submenu_items[i], Font_7x10, White);
        if (i == submenu_index) {
            ssd1306_SetCursor(100, i * 10);
            ssd1306_WriteString("<--", Font_7x10, White);
        }
    }
    ssd1306_UpdateScreen(&hi2c2);
}

void display_frequency() {
    ssd1306_Fill(Black);
    char buff[32];
    snprintf(buff, sizeof(buff), "Freq: %d %s", frequency, units[unit_index]);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString(buff, Font_7x10, White);
    ssd1306_SetCursor(0, 10);
    ssd1306_WriteString("Yellow for Back", Font_7x10, White);
    ssd1306_UpdateScreen(&hi2c2);
}

void display_freq_esp() {
    ssd1306_Fill(Black);
    char buff[32];
    snprintf(buff, sizeof(buff), "Freq ESP: %ld Hz", freq_esp);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString(buff, Font_7x10, White);
    ssd1306_SetCursor(0, 10);
    ssd1306_WriteString("Blue for Gen", Font_7x10, White);
    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("Yellow for Bac```k", Font_7x10, White);
    ssd1306_UpdateScreen(&hi2c2);
}

void setup_clock(int clock_index, int frequency, int unit_index) {
    long freq_hz = frequency;
    switch (unit_index) {
        case 0: // Hz
            freq_hz = frequency;
            break;
        case 1: // kHz
            freq_hz = frequency * 1000;
            break;
        case 2: // MHz
            freq_hz = frequency * 1000000;
            break;
    }

    switch (clock_index) {
        case 0:
            si5351_SetupCLK0(freq_hz, SI5351_DRIVE_STRENGTH_4MA);
            break;
        case 1:
            si5351_SetupCLK1(freq_hz, SI5351_DRIVE_STRENGTH_4MA);
            break;
        case 2:
            si5351_SetupCLK2(freq_hz, SI5351_DRIVE_STRENGTH_4MA);
            break;
    }
    si5351_EnableOutputs((1 << clock_index));
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
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
  HAL_UART_Receive_IT(&huart2, &Rx_temp, 1);
  ssd1306_Init(&hi2c2);
  init_dds();

  // Display ke OLED
  ssd1306_UpdateScreen(&hi2c2);
  ssd1306_SetCursor(7, 0);
  ssd1306_WriteString("Muhammad Hafidzh", Font_7x10, White);
  ssd1306_SetCursor(15, 15);
  ssd1306_WriteString("FP SET", Font_16x26, White);
  ssd1306_SetCursor(30, 50);
  ssd1306_WriteString("5022221069", Font_7x10, White);
  ssd1306_UpdateScreen(&hi2c2);
  HAL_Delay(3000);
  ssd1306_Fill(Black);
  ssd1306_UpdateScreen(&hi2c2);

  // inisiasi menu awal
  display_menu();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

	//merah b2, bir b1, kuning b0, putih a7
    if (menu_active)
    {
        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
        {
            button_biru = 1;
        }
        else
        {
            button_biru = 0;
        }

        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET)
        {
            button_merah = 1;
        }
        else
        {
            button_merah = 0;
        }

        if(button_merah)
        {
            // Scroll
            menu_index = (menu_index + 1) % 3;
            display_menu();
            HAL_Delay(300);
        }

        if(button_biru)
        {
            switch(menu_index)
            {
                case 0:
                    // Disable Clock
                    si5351_EnableOutputs(0);
                    ssd1306_Fill(Black);
                    ssd1306_SetCursor(0, 15);
                    ssd1306_WriteString("Clock Disabled", Font_7x10, White);
                    ssd1306_UpdateScreen(&hi2c2);
                    HAL_Delay(2000);
                    submenu_active = 1;
                    menu_active = 0;
                    active_submenu = 0;
                    display_submenu();
                    break;
                case 1:
                    // Gen CLK ENC
                    submenu_active = 1;
                    menu_active = 0;
                    active_submenu = 1;
                    display_submenu();
                    break;
                case 2:
                    // Gen CLK IOT
                    // si5351_SetupCLK1(20000, SI5351_DRIVE_STRENGTH_4MA);
                    // si5351_EnableOutputs((1<<1));
                    submenu_active = 1;
                    menu_active = 0;
                    active_submenu = 2;
                    display_freq_esp();
                    break;
            }
            HAL_Delay(300);
        }
    }
    else if (submenu_active && active_submenu != 2)
    {
        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
        {
            button_biru = 1;
        }
        else
        {
            button_biru = 0;
        }

        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET)
        {
            button_merah = 1;
        }
        else
        {
            button_merah = 0;
        }

        if(button_merah)
        {
            // Scroll submenu
            int submenu_length = (active_submenu == 1) ? 4 : 1;
            submenu_index = (submenu_index + 1) % submenu_length;
            display_submenu();
            HAL_Delay(300);
        }

        if(button_biru)
        {
            switch(active_submenu)
            {
                case 0:
                    if(submenu_index == 0)
                    {
                        submenu_active = 0;
                        menu_active = 1;
                        display_menu();
                    }
                    break;
                case 1:
                    // Gen CLK ENC submenu
                    switch(submenu_index)
                    {
                        case 0:
                        case 1:
                        case 2:
                            subsubmenu_active = 1;
                            submenu_active = 0;
                            display_frequency();
                            break;
                        case 3:
                            // Back to main menu
                            submenu_active = 0;
                            menu_active = 1;
                            display_menu();
                            break;
                    }
                    break;
                case 2:
                    // Gen CLK IOT submenu
                    display_freq_esp();
                    break;
            }
            HAL_Delay(300);
        }
    }
    else if (subsubmenu_active)
    {
        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
        {
            button_biru = 1;
        }
        else
        {
            button_biru = 0;
        }

        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET)
        {
            button_merah = 1;
        }
        else
        {
            button_merah = 0;
        }

        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET)
        {
            button_putih = 1;
        }
        else
        {
            button_putih = 0;
        }

        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET)
        {
            button_kuning = 1;
        }
        else
        {
            button_kuning = 0;
        }

        // Update frekuensi dan satuan
        frequency = countEnc;
        if(frequency > 100) frequency = 100; // Batas maksimum
        if(frequency < 0) frequency = 0;     // Batas minimum
        display_frequency();

        if(button_putih)
        {
            // ubah unit
            unit_index = (unit_index + 1) % 3;
            display_frequency();
            HAL_Delay(300);
        }

        if(button_kuning)
        {
            // Back
            subsubmenu_active = 0;
            submenu_active = 1;
            display_submenu();
            HAL_Delay(300);
        }

        if(button_biru)
        {
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 15);
            if(submenu_index == 0)
            	ssd1306_WriteString("Generate CLK0 ", Font_7x10, White);
            else if(submenu_index == 1)
            	ssd1306_WriteString("Generate CLK1 ", Font_7x10, White);
            else if(submenu_index == 2)
            	ssd1306_WriteString("Generate CLK2 ", Font_7x10, White);
            ssd1306_UpdateScreen(&hi2c2);
            HAL_Delay(2000);
            submenu_active = 0;
            menu_active = 0;
            active_submenu = 1;
            display_frequency();
            setup_clock(submenu_index, frequency, unit_index);
            HAL_Delay(300);
        }

    }
    else if (active_submenu == 2) // Gen CLK IOT submenu active
    {
        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
        {
            button_biru = 1;
        }
        else
        {
            button_biru = 0;
        }

        if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET)
        {
            button_kuning = 1;
        }
        else
        {
            button_kuning = 0;
        }

        if(button_biru)
        {
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 15);
            ssd1306_WriteString("Generate FreqEsp", Font_7x10, White);
            ssd1306_UpdateScreen(&hi2c2);
            HAL_Delay(2000);
            submenu_active = 1;
            menu_active = 0;
            active_submenu = 2;
            display_freq_esp();
            // Setup clock with freq_esp
//            setup_clock(1, freq_esp, 0); //
            si5351_SetupCLK0(freq_esp, SI5351_DRIVE_STRENGTH_4MA);
            si5351_SetupCLK1(freq_esp*2, SI5351_DRIVE_STRENGTH_4MA);
            si5351_SetupCLK2(freq_esp*10, SI5351_DRIVE_STRENGTH_4MA);
            si5351_EnableOutputs((1<<0) | (1<<1) | (1<<2));
            HAL_Delay(300);
        }

        if(button_kuning)
        {
            submenu_active = 0;
            menu_active = 1;
            display_menu();
            HAL_Delay(300);
        }
    }
//
	  if(HAL_GetTick() - timerLed > 1000)
	  {
		  timerLed = HAL_GetTick();
		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  }

//	  if(HAL_GetTick() - timerEnc > 1)
//	  {
//		  timerEnc = HAL_GetTick();
//		  rotEnc = TIM1 -> CNT;
//	  }
	  // ssd1306_Fill(Black);
	  // char buff[32];
	  // snprintf(buff, sizeof(buff), "ENC : %d", rotEnc);
	  // ssd1306_SetCursor(0, 0);
	  // ssd1306_WriteString(buff, Font_7x10, White);
	  // ssd1306_UpdateScreen(&hi2c2);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2)
    {
        rotEnc = TIM5->CNT;

        countEnc = rotEnc/2;

//        TIM1->CNT = 0;
    }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Master-slave
	if (huart->Instance == USART2)
	{
		if (Rx_temp == 'i' && Rx_state == 0)
		{
			Rx_state = 1;
			Rx_buff[0] = Rx_temp;
			HAL_UART_Receive_IT(&huart2, &Rx_temp, 1);
		}
		else if (Rx_temp == 't' && Rx_state == 1)
		{
			Rx_state = 2;
			Rx_buff[1] = Rx_temp;
			HAL_UART_Receive_IT(&huart2, &Rx_temp, 1);
		}
		else if (Rx_temp == 's' && Rx_state == 2)
		{
			Rx_state = 3;
			Rx_buff[2] = Rx_temp;
			HAL_UART_Receive_IT(&huart2, Rx_buff + 3, 29);
		}
		else if (Rx_state == 3)
		{
			memcpy(&freq_esp, Rx_buff + 3, sizeof(freq_esp));



			Rx_state = 0;
			HAL_UART_Receive_IT(&huart2, &Rx_temp, 1);
		}
		else
		{
			Rx_state = 0;
			HAL_UART_Receive_IT(&huart2, &Rx_temp, 1);
		}
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
