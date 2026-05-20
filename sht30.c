/*
 * sht30.c
 *
 *  Created on: 30 Nis 2026
 *      Author: dilara
 */

#include "sht30.h"

HAL_StatusTypeDef SHT30_Read(I2C_HandleTypeDef *hi2c, SHT30_Data *data)
{
    uint8_t cmd[2] = {0x2C, 0x06};
    uint8_t buf[6];

    // Ölçüm başlat
    if (HAL_I2C_Master_Transmit(hi2c, SHT30_I2C_ADDR, cmd, 2, 100) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(50);

    // Veriyi oku
    if (HAL_I2C_Master_Receive(hi2c, SHT30_I2C_ADDR, buf, 6, 100) != HAL_OK)
        return HAL_ERROR;

    // Sıcaklık hesapla
    uint16_t raw_temp = (buf[0] << 8) | buf[1];
    data->temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);

    // Nem hesapla
    uint16_t raw_hum = (buf[3] << 8) | buf[4];
    data->humidity = 100.0f * ((float)raw_hum / 65535.0f);

    return HAL_OK;
}
