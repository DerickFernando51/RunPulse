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
#include "usb_device.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "string.h"
#include "stm32_lpm.h"
#include "math.h"
#include "max30102.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

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
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

IPCC_HandleTypeDef hipcc;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
volatile uint8_t i2c_done = 0;
volatile uint8_t i2c_busy = 0;
volatile uint8_t usb_ready = 0;
char usb_buffer[64];


#define BUFFER_SIZE     100
#define FS              10

uint32_t red_buf[BUFFER_SIZE];
uint32_t ir_buf[BUFFER_SIZE];
uint8_t  buf_idx = 0;
uint8_t  buf_full = 0;

float    hr_result  = 0.0f;
float    spo2_result = 0.0f;

MAX30102_Sample_t sample;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI1_Init(void);
static void MX_IPCC_Init(void);
static void MX_RTC_Init(void);
static void MX_RF_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char *data = "Hello DERA1\r\n";

#define MAX30102_ADDR (0x57 << 1)
#define MAX17048_ADDR (0x36 << 1)


uint8_t fifo_data[6];
uint32_t red, ir;
char msg[64];


//void MAX30102_Init(void)
//{
//    uint8_t data;
//
//    // 1. RESET
//    data = 0x40;
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x09,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
//    HAL_Delay(100);
//
//    // 2. FIFO Configuration
//    // sample averaging = 4, FIFO rollover enabled, almost full = 17
//    data = 0x4F;
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x08,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
//
//    // 3. Mode Configuration (SpO2 mode = RED + IR)
//    data = 0x03;
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x09,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
//
//    // 4. SpO2 Configuration
//    // ADC range = 4096nA, sample rate = 100Hz, pulse width = 411us (18-bit)
//    data = 0x27;
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x0A,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
//
//    // 5. LED Pulse Amplitude
//    data = 0x24;   // RED LED current
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x0C,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
//
//    data = 0x24;   // IR LED current
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x0D,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
//
//    // 6. Clear FIFO pointers
//    data = 0x00;
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x04,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100); // WR PTR
//
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x05,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100); // OVF CTR
//
//    HAL_I2C_Mem_Write(&hi2c1, MAX30102_ADDR, 0x06,
//                      I2C_MEMADD_SIZE_8BIT, &data, 1, 100); // RD PTR
//}


void MAX30102_Read_FIFO_DMA(void)
{
    if (i2c_busy) return;

    i2c_done = 0;
    i2c_busy = 1;

    HAL_I2C_Mem_Read_DMA(&hi2c1,
                         MAX30102_ADDR,
                         0x07,
                         I2C_MEMADD_SIZE_8BIT,
                         fifo_data,
                         6);
}

//DC removal for HR and spo2
static float mean_u32(uint32_t *buf, int n)
{
    uint64_t sum = 0;
    for (int i = 0; i < n; i++) sum += buf[i];
    return (float)sum / n;
}

//HR calculation using peak detection
static float calc_hr(uint32_t *buf, int n, int fs)
{
    float dc = mean_u32(buf, n);

    // Find peaks in IR signal
    int   peak_count = 0;
    float prev = (float)buf[0] - dc;
    float curr, next;

    for (int i = 1; i < n - 1; i++)
    {
        curr = (float)buf[i]   - dc;
        next = (float)buf[i+1] - dc;

        // Simple peak: rising then falling, above a noise threshold
        if (curr > prev && curr > next && curr > 200.0f)
            peak_count++;

        prev = curr;
    }

    // HR = peaks / time_seconds * 60
    float time_s = (float)n / fs;
    return (peak_count / time_s) * 60.0f;
}

// SpO2 calculation from R ratio
static float calc_spo2(uint32_t *red_b, uint32_t *ir_b, int n)
{
    float dc_red = mean_u32(red_b, n);
    float dc_ir  = mean_u32(ir_b,  n);

    // AC = RMS of AC component
    float ac_red_sq = 0, ac_ir_sq = 0;
    for (int i = 0; i < n; i++)
    {
        float r = (float)red_b[i] - dc_red;
        float ir = (float)ir_b[i] - dc_ir;
        ac_red_sq += r  * r;
        ac_ir_sq  += ir * ir;
    }
    float ac_red = sqrtf(ac_red_sq / n);
    float ac_ir  = sqrtf(ac_ir_sq  / n);

    if (dc_red < 1.0f || dc_ir < 1.0f || ac_ir < 1.0f)
        return -1.0f;  // invalid

    float R = (ac_red / dc_red) / (ac_ir / dc_ir);

    // Empirical calibration
    float spo2 = 110.0f - 25.0f * R;

    if (spo2 > 100.0f) spo2 = 100.0f;
    if (spo2 <  80.0f) spo2 = -1.0f;  // out of range, finger not on sensor

    return spo2;
}


void MAX30102_Process(void)
{
    static uint32_t last_sample = 0;
    static uint8_t samples_since_calc = 0;

    // Trigger DMA read every 100ms
    if (!i2c_busy && (HAL_GetTick() - last_sample >= 100))
    {
        last_sample = HAL_GetTick();
        MAX30102_Read_FIFO_DMA();
    }

    // When DMA read completes
    if (i2c_done)
    {
        i2c_done = 0;

        // Parse raw values
        red = ((uint32_t)(fifo_data[0] & 0x03) << 16) |
              ((uint32_t)fifo_data[1] << 8) | fifo_data[2];
        ir  = ((uint32_t)(fifo_data[3] & 0x03) << 16) |
              ((uint32_t)fifo_data[4] << 8) | fifo_data[5];

        // Finger detection
        if (ir < 50000)
            return;  // no finger, skip

        // Store into ring buffer
        red_buf[buf_idx % BUFFER_SIZE] = red;
        ir_buf[buf_idx % BUFFER_SIZE]  = ir;
        buf_idx++;
        samples_since_calc++;

        // Calculate once buffer is primed, every 10 new samples (1s)
        if (buf_idx >= BUFFER_SIZE && samples_since_calc >= 10)
        {
            samples_since_calc = 0;
            hr_result   = calc_hr(ir_buf, BUFFER_SIZE, FS);
            spo2_result = calc_spo2(red_buf, ir_buf, BUFFER_SIZE);

            snprintf(usb_buffer, sizeof(usb_buffer),
                     "HR: %.1f bpm  SpO2: %.1f%%\r\n", hr_result, spo2_result);
            usb_ready = 1;
        }
    }
}


void MAX17048_Read_VCELL(void)
{
    uint8_t data[2];

    if (HAL_I2C_Mem_Read(&hi2c3,
                         MAX17048_ADDR,
                         0x02,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         2,
                         100) == HAL_OK)
    {
        uint16_t raw = (data[0] << 8) | data[1];
        raw >>= 4;

        uint32_t voltage_mV = (raw * 125) / 100;

        snprintf(usb_buffer, sizeof(usb_buffer),
                 "VCELL: %lu.%03lu V\r\n",
                 voltage_mV / 1000, voltage_mV % 1000);
    }
    else
    {
        snprintf(usb_buffer, sizeof(usb_buffer), "VCELL READ FAIL\r\n");
    }

    usb_ready = 1;
}

void I2C_Check_MAX17048(void)
{
    char msg[64];

    if (HAL_I2C_IsDeviceReady(&hi2c3, MAX17048_ADDR, 3, 100) == HAL_OK)
    {
        sprintf(msg, "MAX17048 FOUND (0x36)\r\n");
    }
    else
    {
        sprintf(msg, "MAX17048 NOT FOUND\r\n");
    }

    CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
}


static void BusyDelay(uint32_t ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < ms);
}

uint8_t USB_IsConnected(void)
{
    return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);
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
  /* Config code for STM32_WPAN (HSE Tuning must be done before system clock configuration) */
  MX_APPE_Config();

  /* USER CODE BEGIN Init */


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* IPCC initialisation */
  MX_IPCC_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_USB_Device_Init();
  MX_RTC_Init();
  MX_RF_Init();
  /* USER CODE BEGIN 2 */


  MAX30102_Init(&hi2c1);
  BusyDelay(3000);


  /* USER CODE END 2 */

  /* Init code for STM32_WPAN */
  MX_APPE_Init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //USB CDC Test
//	  static uint32_t last_print = 0;
//	  	       if (USB_IsConnected() && (HAL_GetTick() - last_print >= 1000))
//	  	       {
//	  	           last_print = HAL_GetTick();
//	  	           CDC_Transmit_FS((uint8_t*)"ALIVE\r\n", 7);
//	  	       }


	  //MAX30102
	  if (MAX30102_ReadSample(&sample) == MAX30102_OK)
	  {
	      if (MAX30102_FingerPresent(&sample))
	      {
	          snprintf(usb_buffer, sizeof(usb_buffer),
	                   "RED: %lu  IR: %lu\r\n",
	                   sample.red, sample.ir);
	          CDC_Transmit_FS((uint8_t*)usb_buffer,
	                          strlen(usb_buffer));
	      }
	  }
	  HAL_Delay(100);


	     // MAX17048
	     static uint32_t last_tick = 0;
	     if (HAL_GetTick() - last_tick > 5000)
	     {
	         last_tick = HAL_GetTick();
	         MAX17048_Read_VCELL();
	     }


    /* USER CODE END WHILE */
    MX_APPE_Process();

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI1
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  LL_HSEM_1StepLock( HSEM, 5 );

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP;
  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_HSE_DIV1024;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10B17DB5;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10B17DB5;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief IPCC Initialization Function
  * @param None
  * @retval None
  */
static void MX_IPCC_Init(void)
{

  /* USER CODE BEGIN IPCC_Init 0 */

  /* USER CODE END IPCC_Init 0 */

  /* USER CODE BEGIN IPCC_Init 1 */

  /* USER CODE END IPCC_Init 1 */
  hipcc.Instance = IPCC;
  if (HAL_IPCC_Init(&hipcc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IPCC_Init 2 */

  /* USER CODE END IPCC_Init 2 */

}

/**
  * @brief RF Initialization Function
  * @param None
  * @retval None
  */
static void MX_RF_Init(void)
{

  /* USER CODE BEGIN RF_Init 0 */

  /* USER CODE END RF_Init 0 */

  /* USER CODE BEGIN RF_Init 1 */

  /* USER CODE END RF_Init 1 */
  /* USER CODE BEGIN RF_Init 2 */

  /* USER CODE END RF_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = CFG_RTC_ASYNCH_PRESCALER;
  hrtc.Init.SynchPrediv = CFG_RTC_SYNCH_PRESCALER;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA2 PA3 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        i2c_done = 1;
        i2c_busy = 0;
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
