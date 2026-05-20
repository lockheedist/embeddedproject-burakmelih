/*
 * bh1750.c
 *
 *  Created on: 7 May 2026
 *      Author: dilara
 */


#include "bh1750.h"

HAL_StatusTypeDef BH1750_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t cmd = 0x10; // Continuous high res mode
    return HAL_I2C_Master_Transmit(hi2c, BH1750_ADDR, &cmd, 1, 100);
}

HAL_StatusTypeDef BH1750_Read(I2C_HandleTypeDef *hi2c, float *lux)
{
    uint8_t buf[2];
    if (HAL_I2C_Master_Receive(hi2c, BH1750_ADDR, buf, 2, 100) != HAL_OK)
        return HAL_ERROR;

    uint16_t raw = (buf[0] << 8) | buf[1];
    *lux = raw / 1.2f;
    return HAL_OK;
}
