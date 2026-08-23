#ifndef HC05_H
#define HC05_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef struct
{
    UART_HandleTypeDef *huart;

} HC05_t;


void HC05_Init(HC05_t *hc05,
               UART_HandleTypeDef *huart);

HAL_StatusTypeDef HC05_Send(HC05_t *hc05,
                            const uint8_t *data,
                            uint16_t length);

HAL_StatusTypeDef HC05_Receive(HC05_t *hc05,
                               uint8_t *data,
                               uint16_t length,
                               uint32_t timeout);

#endif /* HC05_H */