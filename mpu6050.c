#include "mpu6050.h"

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t buf[2];
    // Sleep modundan çıkar
    buf[0] = 0x6B;
    buf[1] = 0x00;
    return HAL_I2C_Master_Transmit(hi2c, MPU6050_ADDR, buf, 2, 100);
}

HAL_StatusTypeDef MPU6050_Read(I2C_HandleTypeDef *hi2c, MPU6050_Data *data)
{
    uint8_t buf[14];
    uint8_t reg = 0x3B;

    if (HAL_I2C_Master_Transmit(hi2c, MPU6050_ADDR, &reg, 1, 100) != HAL_OK)
        return HAL_ERROR;

    if (HAL_I2C_Master_Receive(hi2c, MPU6050_ADDR, buf, 14, 100) != HAL_OK)
        return HAL_ERROR;

    int16_t ax = (buf[0] << 8) | buf[1];
    int16_t ay = (buf[2] << 8) | buf[3];
    int16_t az = (buf[4] << 8) | buf[5];
    int16_t gx = (buf[8] << 8) | buf[9];
    int16_t gy = (buf[10] << 8) | buf[11];
    int16_t gz = (buf[12] << 8) | buf[13];

    data->accel_x = ax / 16384.0f;
    data->accel_y = ay / 16384.0f;
    data->accel_z = az / 16384.0f;
    data->gyro_x  = gx / 131.0f;
    data->gyro_y  = gy / 131.0f;
    data->gyro_z  = gz / 131.0f;

    return HAL_OK;
}

