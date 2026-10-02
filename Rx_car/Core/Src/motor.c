#include "motor.h"
#include "main.h"

#define MOTOR_PWM_MAX 99


static int16_t clamp_control(int16_t value)
{
    if (value > 100)
    {
        return 100;
    }

    if (value < -100)
    {
        return -100;
    }

    return value;
}


static void Motor_SetOne(Motor_t *motor,
                         uint32_t channel,
                         GPIO_TypeDef *in1_port,
                         uint16_t in1_pin,
                         GPIO_TypeDef *in2_port,
                         uint16_t in2_pin,
                         int16_t value)
{
    value = clamp_control(value);

    uint16_t pwm = 0;

    if (value > 0)
    {
        /* Forward */
        HAL_GPIO_WritePin(in1_port,
                          in1_pin,
                          GPIO_PIN_SET);

        HAL_GPIO_WritePin(in2_port,
                          in2_pin,
                          GPIO_PIN_RESET);

        pwm = (uint16_t)value;
    }
    else if (value < 0)
    {
        /* Reverse */
        HAL_GPIO_WritePin(in1_port,
                          in1_pin,
                          GPIO_PIN_RESET);

        HAL_GPIO_WritePin(in2_port,
                          in2_pin,
                          GPIO_PIN_SET);

        pwm = (uint16_t)(-value);
    }
    else
    {
        /* Stop */
        HAL_GPIO_WritePin(in1_port,
                          in1_pin,
                          GPIO_PIN_RESET);

        HAL_GPIO_WritePin(in2_port,
                          in2_pin,
                          GPIO_PIN_RESET);

        pwm = 0;
    }

    uint32_t compare =
        ((uint32_t)pwm * MOTOR_PWM_MAX) / 100;

    __HAL_TIM_SET_COMPARE(motor->htim,
                          channel,
                          compare);
}


HAL_StatusTypeDef Motor_Init(Motor_t *motor,
                             TIM_HandleTypeDef *htim)
{
    if (motor == NULL || htim == NULL)
    {
        return HAL_ERROR;
    }

    motor->htim = htim;

    /* Start PWM on all 4 channels */
    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_2) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_3) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_TIM_PWM_Start(htim, TIM_CHANNEL_4) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // /* Enable both TB6612 */
    // HAL_GPIO_WritePin(STBY_GPIO_Port,
    //                   STBY_Pin,
    //                   GPIO_PIN_SET);

    Motor_Stop(motor);

    return HAL_OK;
}


void Motor_Stop(Motor_t *motor)
{
    /* Left Front - TB1 A */
    Motor_SetOne(motor,
                 TIM_CHANNEL_1,
                 A1IN1_GPIO_Port, A1IN1_Pin,
                 A1IN2_GPIO_Port, A1IN2_Pin,
                 0);

    /* Right Front - TB1 B */
    Motor_SetOne(motor,
                 TIM_CHANNEL_2,
                 B1IN1_GPIO_Port, B1IN1_Pin,
                 B1IN2_GPIO_Port, B1IN2_Pin,
                 0);

    /* Left Rear - TB2 A */
    Motor_SetOne(motor,
                 TIM_CHANNEL_3,
                 A2IN1_GPIO_Port, A2IN1_Pin,
                 A2IN2_GPIO_Port, A2IN2_Pin,
                 0);

    /* Right Rear - TB2 B */
    Motor_SetOne(motor,
                 TIM_CHANNEL_4,
                 B2IN1_GPIO_Port, B2IN1_Pin,
                 B2IN2_GPIO_Port, B2IN2_Pin,
                 0);
}


#define CONTROL_MAX_TURN 80

void Motor_SetControl(Motor_t *motor,
                      int16_t forward,
                      int16_t turn)
{
    int16_t left = 0;
    int16_t right = 0;

    if (forward > 0)
    {
        /* --- TIẾN (FORWARD / FORWARD-CURVE) --- */
        if (turn > 0)
        {
            /* Tiến - Phải: bánh trái đi nhanh (bánh ngoài), bánh phải giảm tốc (bánh trong) */
            int16_t turn_factor = CONTROL_MAX_TURN - turn;
            if (turn_factor < 0) turn_factor = 0;

            left = forward;
            right = (int16_t)(((int32_t)forward * turn_factor) / CONTROL_MAX_TURN);
        }
        else if (turn < 0)
        {
            /* Tiến - Trái: bánh phải đi nhanh (bánh ngoài), bánh trái giảm tốc (bánh trong) */
            int16_t turn_factor = CONTROL_MAX_TURN - (-turn);
            if (turn_factor < 0) turn_factor = 0;

            right = forward;
            left = (int16_t)(((int32_t)forward * turn_factor) / CONTROL_MAX_TURN);
        }
        else
        {
            /* Tiến thẳng */
            left = forward;
            right = forward;
        }
    }
    else if (forward < 0)
    {
        /* --- LÙI (BACKWARD / BACKWARD-CURVE) --- */
        if (turn > 0)
        {
            /* Lùi - Phải: cả 2 cùng lùi, bánh phải lùi chậm hơn */
            int16_t turn_factor = CONTROL_MAX_TURN - turn;
            if (turn_factor < 0) turn_factor = 0;

            left = forward;
            right = (int16_t)(((int32_t)forward * turn_factor) / CONTROL_MAX_TURN);
        }
        else if (turn < 0)
        {
            /* Lùi - Trái: cả 2 cùng lùi, bánh trái lùi chậm hơn */
            int16_t turn_factor = CONTROL_MAX_TURN - (-turn);
            if (turn_factor < 0) turn_factor = 0;

            right = forward;
            left = (int16_t)(((int32_t)forward * turn_factor) / CONTROL_MAX_TURN);
        }
        else
        {
            /* Lùi thẳng */
            left = forward;
            right = forward;
        }
    }
    else
    {
        /* --- XOAY TẠI CHỖ (SPIN TURN / STOP) --- */
        if (turn > 0)
        {
            /* Xoay phải tại chỗ */
            left = turn;
            right = -turn;
        }
        else if (turn < 0)
        {
            /* Xoay trái tại chỗ */
            left = turn;
            right = -turn;
        }
        else
        {
            /* Dừng hẳn */
            left = 0;
            right = 0;
        }
    }

    left = clamp_control(left);
    right = clamp_control(right);

    /* Left Front */
    Motor_SetOne(motor,
                 TIM_CHANNEL_1,
                 A1IN1_GPIO_Port, A1IN1_Pin,
                 A1IN2_GPIO_Port, A1IN2_Pin,
                 left);

    /* Right Front */
    Motor_SetOne(motor,
                 TIM_CHANNEL_2,
                 B1IN1_GPIO_Port, B1IN1_Pin,
                 B1IN2_GPIO_Port, B1IN2_Pin,
                 right);

    /* Left Rear */
    Motor_SetOne(motor,
                 TIM_CHANNEL_3,
                 A2IN1_GPIO_Port, A2IN1_Pin,
                 A2IN2_GPIO_Port, A2IN2_Pin,
                 left);

    /* Right Rear */
    Motor_SetOne(motor,
                 TIM_CHANNEL_4,
                 B2IN1_GPIO_Port, B2IN1_Pin,
                 B2IN2_GPIO_Port, B2IN2_Pin,
                 right);
}

void Motor_Forward(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, speed, 0);
}

void Motor_Backward(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, -speed, 0);
}

void Motor_SpinLeft(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, 0, -speed);
}

void Motor_SpinRight(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, 0, speed);
}

void Motor_ForwardLeft(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, speed, -(speed * 6 / 10));
}

void Motor_ForwardRight(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, speed, (speed * 6 / 10));
}

void Motor_BackwardLeft(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, -speed, -(speed * 6 / 10));
}

void Motor_BackwardRight(Motor_t *motor, int16_t speed)
{
    Motor_SetControl(motor, -speed, (speed * 6 / 10));
}

CarDirection_t Motor_GetDirection(int16_t forward, int16_t turn)
{
    const int16_t f_thresh = 15;
    const int16_t t_thresh = 15;

    if (forward > f_thresh && turn > t_thresh)
    {
        return CAR_FORWARD_RIGHT;
    }
    if (forward > f_thresh && turn < -t_thresh)
    {
        return CAR_FORWARD_LEFT;
    }
    if (forward < -f_thresh && turn > t_thresh)
    {
        return CAR_BACKWARD_RIGHT;
    }
    if (forward < -f_thresh && turn < -t_thresh)
    {
        return CAR_BACKWARD_LEFT;
    }
    if (forward > f_thresh)
    {
        return CAR_FORWARD;
    }
    if (forward < -f_thresh)
    {
        return CAR_BACKWARD;
    }
    if (turn > t_thresh)
    {
        return CAR_TURN_RIGHT;
    }
    if (turn < -t_thresh)
    {
        return CAR_TURN_LEFT;
    }

    return CAR_STOP;
}


void Motor_TestSingle(Motor_t *motor,
                      MotorId_t id,
                      int16_t speed)
{
    Motor_Stop(motor);

    switch (id)
    {
        case MOTOR_LF:
            Motor_SetOne(motor,
                         TIM_CHANNEL_1,
                         A1IN1_GPIO_Port, A1IN1_Pin,
                         A1IN2_GPIO_Port, A1IN2_Pin,
                         speed);
            break;

        case MOTOR_RF:
            Motor_SetOne(motor,
                         TIM_CHANNEL_2,
                         B1IN1_GPIO_Port, B1IN1_Pin,
                         B1IN2_GPIO_Port, B1IN2_Pin,
                         speed);
            break;

        case MOTOR_LR:
            Motor_SetOne(motor,
                         TIM_CHANNEL_3,
                         A2IN1_GPIO_Port, A2IN1_Pin,
                         A2IN2_GPIO_Port, A2IN2_Pin,
                         speed);
            break;

        case MOTOR_RR:
            Motor_SetOne(motor,
                         TIM_CHANNEL_4,
                         B2IN1_GPIO_Port, B2IN1_Pin,
                         B2IN2_GPIO_Port, B2IN2_Pin,
                         speed);
            break;

        default:
            Motor_Stop(motor);
            break;
    }
}