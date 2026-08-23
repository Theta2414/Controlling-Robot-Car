#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f1xx_hal.h"
#include <stdint.h>


typedef struct
{
    TIM_HandleTypeDef *htim;

} Motor_t;


typedef enum
{
    MOTOR_LF = 0,
    MOTOR_RF,
    MOTOR_LR,
    MOTOR_RR

} MotorId_t;


HAL_StatusTypeDef Motor_Init(Motor_t *motor,
                             TIM_HandleTypeDef *htim);

void Motor_SetControl(Motor_t *motor,
                      int16_t forward,
                      int16_t turn);

void Motor_Stop(Motor_t *motor);

void Motor_TestSingle(Motor_t *motor,
                      MotorId_t id,
                      int16_t speed);


#endif /* MOTOR_H */