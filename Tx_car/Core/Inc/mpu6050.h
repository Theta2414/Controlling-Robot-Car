#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef enum
{
    MPU6050_OK = 0,
    MPU6050_ERROR
} MPU6050_Status;


typedef struct
{
    /* Hardware */
    I2C_HandleTypeDef *hi2c;

    /* Raw data */
    int16_t accel_x_raw;
    int16_t accel_y_raw;
    int16_t accel_z_raw;

    int16_t gyro_x_raw;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;

    /* Converted data */
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;

    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;

    /* Accelerometer angles */
    float pitch_acc;
    float roll_acc;

    /* Calibration */
    float gyro_x_bias;
    float gyro_y_bias;
    float gyro_z_bias;

    float pitch_offset;
    float roll_offset;

} MPU6050_t;


MPU6050_Status MPU6050_Init(MPU6050_t *mpu,
                            I2C_HandleTypeDef *hi2c);

MPU6050_Status MPU6050_Read(MPU6050_t *mpu);

MPU6050_Status MPU6050_Calibrate(MPU6050_t *mpu);


#endif /* MPU6050_H */