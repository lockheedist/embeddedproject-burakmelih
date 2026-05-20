/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sht30.h"
#include "bh1750.h"
#include "mpu6050.h"
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* USER CODE BEGIN Variables */
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;

typedef struct {
    float temperature;
    float humidity;
    float lux;
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
} SensorData_t;

volatile SensorData_t g_sensor_data = {0};

#define TEMP_THRESHOLD  30.0f
#define HUM_THRESHOLD   70.0f
#define LUX_THRESHOLD   10.0f

FATFS g_fs;
FIL   g_fil;
/* USER CODE END Variables */

/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN Application */

void StartSensorTask(void *argument)
{
    SHT30_Data sht30;
    MPU6050_Data mpu;
    float lux;

    BH1750_Init(&hi2c1);
    MPU6050_Init(&hi2c1);
    osDelay(200);

    for(;;)
    {
        if (SHT30_Read(&hi2c1, &sht30) == HAL_OK) {
            g_sensor_data.temperature = sht30.temperature;
            g_sensor_data.humidity    = sht30.humidity;
        }
        if (BH1750_Read(&hi2c1, &lux) == HAL_OK) {
            g_sensor_data.lux = lux;
        }
        if (MPU6050_Read(&hi2c1, &mpu) == HAL_OK) {
            g_sensor_data.accel_x = mpu.accel_x;
            g_sensor_data.accel_y = mpu.accel_y;
            g_sensor_data.accel_z = mpu.accel_z;
            g_sensor_data.gyro_x  = mpu.gyro_x;
            g_sensor_data.gyro_y  = mpu.gyro_y;
            g_sensor_data.gyro_z  = mpu.gyro_z;
        }
        osDelay(500);
    }
}

void StartLogTask(void *argument)
{
    UINT bw;
    char sd_buf[150];

    osDelay(1000);

    for(;;)
    {
        if (f_open(&g_fil, "data.csv", FA_WRITE | FA_OPEN_APPEND) == FR_OK)
        {
            uint32_t uptime_sec = osKernelGetTickCount() / 1000;
            uint32_t hours   = uptime_sec / 3600;
            uint32_t minutes = (uptime_sec % 3600) / 60;
            uint32_t seconds = uptime_sec % 60;

            sprintf(sd_buf, "%02lu:%02lu:%02lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                    hours, minutes, seconds,
                    g_sensor_data.temperature, g_sensor_data.humidity,
                    g_sensor_data.lux,
                    g_sensor_data.accel_x, g_sensor_data.accel_y, g_sensor_data.accel_z,
                    g_sensor_data.gyro_x, g_sensor_data.gyro_y, g_sensor_data.gyro_z);
            f_write(&g_fil, sd_buf, strlen(sd_buf), &bw);
            f_close(&g_fil);
        }
        osDelay(2000);
    }
}

void StartAlertTask(void *argument)
{
    for(;;)
    {
        if (g_sensor_data.temperature > TEMP_THRESHOLD ||
            g_sensor_data.humidity    > HUM_THRESHOLD  ||
            g_sensor_data.lux        < LUX_THRESHOLD)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
        }
        osDelay(500);
    }
}

void StartUartTask(void *argument)
{
    char uart_buf[128];
    osDelay(1000);

    for(;;)
    {
        sprintf(uart_buf,
                "Temp:%.2fC Hum:%.2f%% Lux:%.2f Ax:%.2f Ay:%.2f Az:%.2f Gx:%.2f Gy:%.2f Gz:%.2f\r\n",
                g_sensor_data.temperature, g_sensor_data.humidity,
                g_sensor_data.lux,
                g_sensor_data.accel_x, g_sensor_data.accel_y, g_sensor_data.accel_z,
                g_sensor_data.gyro_x, g_sensor_data.gyro_y, g_sensor_data.gyro_z);
        HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 100);
        osDelay(1000);
    }
}

/* USER CODE END Application */
