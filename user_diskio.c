#include <string.h>
#include <stdio.h>
#include "ff_gen_drv.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;

static volatile DSTATUS Stat = STA_NOINIT;
static uint8_t CardType;

#define SD_CS_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)
#define SD_CS_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)

DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif

Diskio_drvTypeDef USER_Driver = {
  USER_initialize, USER_status, USER_read,
#if _USE_WRITE
  USER_write,
#endif
#if _USE_IOCTL == 1
  USER_ioctl,
#endif
};

static uint8_t SPI_RW(uint8_t data) {
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, 10);
    return rx;
}

static void SPI_Release(void) { SPI_RW(0xFF); }

static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg) {
    uint8_t res, n;
    SPI_Release();
    SPI_RW(cmd | 0x40);
    SPI_RW((uint8_t)(arg >> 24));
    SPI_RW((uint8_t)(arg >> 16));
    SPI_RW((uint8_t)(arg >> 8));
    SPI_RW((uint8_t)(arg));
    n = 0x01;
    if (cmd == 0) n = 0x95;
    if (cmd == 8) n = 0x87;
    SPI_RW(n);
    n = 10;
    do { res = SPI_RW(0xFF); } while ((res & 0x80) && --n);
    return res;
}

DSTATUS USER_initialize(BYTE pdrv) {
    uint8_t n, cmd, ty, ocr[4];
    uint32_t tmr;

    SD_CS_HIGH();
    for (n = 10; n; n--) SPI_RW(0xFF);

    ty = 0;
    SD_CS_LOW();

    if (SD_SendCmd(0, 0) == 1) {
        if (SD_SendCmd(8, 0x1AA) == 1) {
            for (n = 0; n < 4; n++) ocr[n] = SPI_RW(0xFF);
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
                tmr = 1000;
                do {
                    SD_CS_HIGH(); SPI_RW(0xFF); SD_CS_LOW();
                    SD_SendCmd(55, 0);
                    tmr--;
                } while (SD_SendCmd(41, 0x40000000) && tmr);
                if (tmr && SD_SendCmd(58, 0) == 0) {
                    for (n = 0; n < 4; n++) ocr[n] = SPI_RW(0xFF);
                    ty = (ocr[0] & 0x40) ? 6 : 2;
                }
            }
        } else {
            SD_CS_HIGH(); SPI_RW(0xFF); SD_CS_LOW();
            SD_SendCmd(55, 0);
            if (SD_SendCmd(41, 0) <= 1) { ty = 2; cmd = 41; }
            else { ty = 1; cmd = 1; }
            tmr = 1000;
            do {
                if (ty == 2) { SD_CS_HIGH(); SPI_RW(0xFF); SD_CS_LOW(); SD_SendCmd(55, 0); }
                tmr--;
            } while (SD_SendCmd(cmd, 0) && tmr);
            if (!tmr || SD_SendCmd(16, 512) != 0) ty = 0;
        }
    }
    CardType = ty;
    SD_CS_HIGH();
    SPI_RW(0xFF);
    Stat = ty ? 0 : STA_NOINIT;
    return Stat;
}

DSTATUS USER_status(BYTE pdrv) { return Stat; }

DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (!(CardType & 4)) sector *= 512;
    SD_CS_LOW();
    if (count == 1) {
        if (SD_SendCmd(17, sector) == 0) {
            uint16_t tmr = 5000;
            while (SPI_RW(0xFF) != 0xFE && --tmr);
            if (tmr) {
                for (int i = 0; i < 512; i++) buff[i] = SPI_RW(0xFF);
                SPI_RW(0xFF); SPI_RW(0xFF);
                SD_CS_HIGH(); SPI_RW(0xFF);
                return RES_OK;
            }
        }
    }
    SD_CS_HIGH(); SPI_RW(0xFF);
    return RES_ERROR;
}

#if _USE_WRITE == 1
DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (!(CardType & 4)) sector *= 512;
    SD_CS_LOW();
    if (count == 1) {
        if (SD_SendCmd(24, sector) == 0) {
            SPI_RW(0xFE);
            for (int i = 0; i < 512; i++) SPI_RW(buff[i]);
            SPI_RW(0xFF); SPI_RW(0xFF);
            uint8_t res = SPI_RW(0xFF);
            if ((res & 0x1F) == 0x05) {
                uint16_t tmr = 5000;
                while (!SPI_RW(0xFF) && --tmr);
                SD_CS_HIGH(); SPI_RW(0xFF);
                return tmr ? RES_OK : RES_ERROR;
            }
        }
    }
    SD_CS_HIGH(); SPI_RW(0xFF);
    return RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    DRESULT res = RES_ERROR;
    switch (cmd) {
        case CTRL_SYNC:
            SD_CS_LOW();
            { uint16_t t = 5000; while (!SPI_RW(0xFF) && --t); }
            SD_CS_HIGH();
            res = RES_OK; break;
        case GET_SECTOR_SIZE: *(WORD*)buff = 512; res = RES_OK; break;
        case GET_SECTOR_COUNT: *(DWORD*)buff = 32768; res = RES_OK; break;
        case GET_BLOCK_SIZE: *(DWORD*)buff = 1; res = RES_OK; break;
    }
    return res;
}
#endif
