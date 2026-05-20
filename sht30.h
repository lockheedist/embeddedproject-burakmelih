/*
 * sht30.h
 *
 *  Created on: 30 __Nis__ 2026
 *      Author: __dilara__
 */

#ifndef INC_SHT30_H_
#define INC_SHT30_H_

#include "stm32f4xx_hal.h"

#define SHT30_I2C_ADDR  (0x44 << 1)

typedef struct {
    float temperature;
    float humidity;
} SHT30_Data;

HAL_StatusTypeDef SHT30_Read(I2C_HandleTypeDef *hi2c, SHT30_Data *data);

#endif /* INC_SHT30_H_ */
