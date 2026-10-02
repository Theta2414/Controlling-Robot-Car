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


typedef enum
{
    CAR_STOP = 0,
    CAR_FORWARD,
    CAR_BACKWARD,
    CAR_TURN_LEFT,
    CAR_TURN_RIGHT,
    CAR_FORWARD_LEFT,
    CAR_FORWARD_RIGHT,
    CAR_BACKWARD_LEFT,
    CAR_BACKWARD_RIGHT

} CarDirection_t;


HAL_StatusTypeDef Motor_Init(Motor_t *motor,
                             TIM_HandleTypeDef *htim);

void Motor_SetControl(Motor_t *motor,
                      int16_t forward,
                      int16_t turn);

void Motor_Stop(Motor_t *motor);

/* Direct directional control functions */
void Motor_Forward(Motor_t *motor, int16_t speed);
void Motor_Backward(Motor_t *motor, int16_t speed);
void Motor_SpinLeft(Motor_t *motor, int16_t speed);
void Motor_SpinRight(Motor_t *motor, int16_t speed);

/* 4 diagonal curve movement functions */
void Motor_ForwardLeft(Motor_t *motor, int16_t speed);
void Motor_ForwardRight(Motor_t *motor, int16_t speed);
void Motor_BackwardLeft(Motor_t *motor, int16_t speed);
void Motor_BackwardRight(Motor_t *motor, int16_t speed);

CarDirection_t Motor_GetDirection(int16_t forward, int16_t turn);

void Motor_TestSingle(Motor_t *motor,
                      MotorId_t id,
                      int16_t speed);


#endif /* MOTOR_H */