#include "hc05.h"


void HC05_Init(HC05_t *hc05,
               UART_HandleTypeDef *huart)
{
    if (hc05 == NULL)
    {
        return;
    }

    hc05->huart = huart;
}


HAL_StatusTypeDef HC05_Send(HC05_t *hc05,
                            const uint8_t *data,
                            uint16_t length)
{
    if (hc05 == NULL ||
        hc05->huart == NULL ||
        data == NULL ||
        length == 0)
    {
        return HAL_ERROR;
    }

    return HAL_UART_Transmit(hc05->huart,
                             (uint8_t *)data,
                             length,
                             100);
}


HAL_StatusTypeDef HC05_Receive(HC05_t *hc05,
                               uint8_t *data,
                               uint16_t length,
                               uint32_t timeout)
{
    if (hc05 == NULL ||
        hc05->huart == NULL ||
        data == NULL ||
        length == 0)
    {
        return HAL_ERROR;
    }

    return HAL_UART_Receive(hc05->huart,
                            data,
                            length,
                            timeout);
}