#include "mpu6050.h"
#include <math.h>

/* MPU6050 address */
#define MPU6050_ADDR            (0x68 << 1)

/* Registers */
#define MPU6050_REG_WHO_AM_I    0x75
#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_SMPLRT_DIV  0x19
#define MPU6050_REG_CONFIG      0x1A
#define MPU6050_REG_GYRO_CONFIG 0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C

/* Our module returns this */
#define MPU6050_WHO_AM_I_VALUE  0x72

/*Values*/
#define MPU6050_REG_ACCEL_XOUT_H   0x3B

#define MPU6050_ACCEL_SENSITIVITY  16384.0f
#define MPU6050_GYRO_SENSITIVITY   131.0f

#define RAD_TO_DEG                 57.295779513f

static float wrap_angle(float angle)
{
    while (angle > 180.0f)
    {
        angle -= 360.0f;
    }

    while (angle < -180.0f)
    {
        angle += 360.0f;
    }

    return angle;
}

MPU6050_Status MPU6050_Init(MPU6050_t *mpu,
                            I2C_HandleTypeDef *hi2c)
{
    uint8_t data;
    uint8_t who_am_i;

    if (mpu == NULL || hi2c == NULL)
    {
        return MPU6050_ERROR;
    }

    /* Save I2C handle */
    mpu->hi2c = hi2c;


    /* Check device exists */
    if (HAL_I2C_IsDeviceReady(mpu->hi2c,
                              MPU6050_ADDR,
                              3,
                              100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }


    /* Read WHO_AM_I */
    if (HAL_I2C_Mem_Read(mpu->hi2c,
                         MPU6050_ADDR,
                         MPU6050_REG_WHO_AM_I,
                         I2C_MEMADD_SIZE_8BIT,
                         &who_am_i,
                         1,
                         100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }

    if (who_am_i != MPU6050_WHO_AM_I_VALUE)
    {
        return MPU6050_ERROR;
    }


    /* Wake sensor */
    data = 0x00;

    if (HAL_I2C_Mem_Write(mpu->hi2c,
                          MPU6050_ADDR,
                          MPU6050_REG_PWR_MGMT_1,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }

    HAL_Delay(100);


    /* Gyroscope: ±250 deg/s */
    data = 0x00;

    if (HAL_I2C_Mem_Write(mpu->hi2c,
                          MPU6050_ADDR,
                          MPU6050_REG_GYRO_CONFIG,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }


    /* Accelerometer: ±2g */
    data = 0x00;

    if (HAL_I2C_Mem_Write(mpu->hi2c,
                          MPU6050_ADDR,
                          MPU6050_REG_ACCEL_CONFIG,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }


    /* Sample rate = 100 Hz */
    data = 9;

    if (HAL_I2C_Mem_Write(mpu->hi2c,
                          MPU6050_ADDR,
                          MPU6050_REG_SMPLRT_DIV,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }


    /* Digital low-pass filter */
    data = 0x03;

    if (HAL_I2C_Mem_Write(mpu->hi2c,
                          MPU6050_ADDR,
                          MPU6050_REG_CONFIG,
                          I2C_MEMADD_SIZE_8BIT,
                          &data,
                          1,
                          100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }


    /* Initial calibration values */
    mpu->gyro_x_bias = 0.0f;
    mpu->gyro_y_bias = 0.0f;
    mpu->gyro_z_bias = 0.0f;

    mpu->pitch_offset = 0.0f;
    mpu->roll_offset = 0.0f;

    return MPU6050_OK;
}


MPU6050_Status MPU6050_Read(MPU6050_t *mpu)
{
    uint8_t data[14];

    if (mpu == NULL || mpu->hi2c == NULL)
    {
        return MPU6050_ERROR;
    }

    /* Read 14 consecutive bytes:
     *
     * 0-1   Accel X
     * 2-3   Accel Y
     * 4-5   Accel Z
     * 6-7   Temperature
     * 8-9   Gyro X
     * 10-11 Gyro Y
     * 12-13 Gyro Z
     */
    if (HAL_I2C_Mem_Read(mpu->hi2c,
                         MPU6050_ADDR,
                         MPU6050_REG_ACCEL_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         14,
                         100) != HAL_OK)
    {
        return MPU6050_ERROR;
    }

    /* =====================================================
     * RAW ACCELEROMETER
     * ===================================================== */

    mpu->accel_x_raw =
        (int16_t)(((uint16_t)data[0] << 8) | data[1]);

    mpu->accel_y_raw =
        (int16_t)(((uint16_t)data[2] << 8) | data[3]);

    mpu->accel_z_raw =
        (int16_t)(((uint16_t)data[4] << 8) | data[5]);


    /* =====================================================
     * RAW GYROSCOPE
     * ===================================================== */

    mpu->gyro_x_raw =
        (int16_t)(((uint16_t)data[8] << 8) | data[9]);

    mpu->gyro_y_raw =
        (int16_t)(((uint16_t)data[10] << 8) | data[11]);

    mpu->gyro_z_raw =
        (int16_t)(((uint16_t)data[12] << 8) | data[13]);


    /* =====================================================
     * ACCEL RAW -> g
     * ±2g => 16384 LSB/g
     * ===================================================== */

    mpu->accel_x_g =
        (float)mpu->accel_x_raw / MPU6050_ACCEL_SENSITIVITY;

    mpu->accel_y_g =
        (float)mpu->accel_y_raw / MPU6050_ACCEL_SENSITIVITY;

    mpu->accel_z_g =
        (float)mpu->accel_z_raw / MPU6050_ACCEL_SENSITIVITY;


    /* =====================================================
     * GYRO RAW -> deg/s
     * ±250 deg/s => 131 LSB/(deg/s)
     * ===================================================== */

    mpu->gyro_x_dps =
        (float)mpu->gyro_x_raw / MPU6050_GYRO_SENSITIVITY
        - mpu->gyro_x_bias;

    mpu->gyro_y_dps =
        (float)mpu->gyro_y_raw / MPU6050_GYRO_SENSITIVITY
        - mpu->gyro_y_bias;

    mpu->gyro_z_dps =
        (float)mpu->gyro_z_raw / MPU6050_GYRO_SENSITIVITY
        - mpu->gyro_z_bias;


    /* =====================================================
     * ACCELEROMETER -> ROLL / PITCH
     * ===================================================== */

    mpu->roll_acc =
        atan2f(mpu->accel_y_g,
               mpu->accel_z_g)
        * RAD_TO_DEG;

    mpu->pitch_acc =
        atan2f(-mpu->accel_x_g,
               sqrtf(
                   mpu->accel_y_g * mpu->accel_y_g +
                   mpu->accel_z_g * mpu->accel_z_g
               ))
        * RAD_TO_DEG;


    /* Apply calibration offsets */
    mpu->roll_acc =
        wrap_angle(mpu->roll_acc - mpu->roll_offset);

    mpu->pitch_acc =
        mpu->pitch_acc - mpu->pitch_offset;


    return MPU6050_OK;
}


MPU6050_Status MPU6050_Calibrate(MPU6050_t *mpu)
{
    const uint16_t samples = 200;

    float gyro_x_sum = 0.0f;
    float gyro_y_sum = 0.0f;
    float gyro_z_sum = 0.0f;

    float pitch_sum = 0.0f;

    float roll_sin_sum = 0.0f;
    float roll_cos_sum = 0.0f;

    uint16_t valid_samples = 0;


    if (mpu == NULL || mpu->hi2c == NULL)
    {
        return MPU6050_ERROR;
    }


    /*
     * Remove old calibration before recalibrating.
     * Otherwise MPU6050_Read() would subtract the previous values.
     */
    mpu->gyro_x_bias = 0.0f;
    mpu->gyro_y_bias = 0.0f;
    mpu->gyro_z_bias = 0.0f;

    mpu->pitch_offset = 0.0f;
    mpu->roll_offset = 0.0f;


    for (uint16_t i = 0; i < samples; i++)
    {
        if (MPU6050_Read(mpu) == MPU6050_OK)
        {
            /* Gyro bias */
            gyro_x_sum += mpu->gyro_x_dps;
            gyro_y_sum += mpu->gyro_y_dps;
            gyro_z_sum += mpu->gyro_z_dps;


            /* Pitch offset */
            pitch_sum += mpu->pitch_acc;


            /*
             * Circular average for roll.
             *
             * Example:
             * +179° and -179° should average near 180°,
             * not 0°.
             */
            float roll_rad =
                mpu->roll_acc / RAD_TO_DEG;

            roll_sin_sum += sinf(roll_rad);
            roll_cos_sum += cosf(roll_rad);


            valid_samples++;
        }

        HAL_Delay(10);
    }


    if (valid_samples == 0)
    {
        return MPU6050_ERROR;
    }


    /* Gyroscope biases */
    mpu->gyro_x_bias =
        gyro_x_sum / valid_samples;

    mpu->gyro_y_bias =
        gyro_y_sum / valid_samples;

    mpu->gyro_z_bias =
        gyro_z_sum / valid_samples;


    /* Pitch neutral position */
    mpu->pitch_offset =
        pitch_sum / valid_samples;


    /* Roll neutral position */
    mpu->roll_offset =
        atan2f(roll_sin_sum,
               roll_cos_sum)
        * RAD_TO_DEG;


    return MPU6050_OK;
}

