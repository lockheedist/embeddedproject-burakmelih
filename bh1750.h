/*
 * bh1750.h
 *
 *  Created on: 7 May 2026
 *      Author: dilara
 */

#ifndef INC_BH1750_H_
#define INC_BH1750_H_

#include "stm32f4xx_hal.h"

#define BH1750_ADDR  (0x23 << 1)

HAL_StatusTypeDef BH1750_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BH1750_Read(I2C_HandleTypeDef *hi2c, float *lux);

#endif
