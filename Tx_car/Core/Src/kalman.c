#include "kalman.h"

void Kalman_Init(Kalman_t *kalman)
{
    kalman->angle = 0.0f;
    kalman->bias = 0.0f;
    kalman->rate = 0.0f;

    kalman->P[0][0] = 0.0f;
    kalman->P[0][1] = 0.0f;
    kalman->P[1][0] = 0.0f;
    kalman->P[1][1] = 0.0f;

    kalman->Q_angle = 0.001f;
    kalman->Q_bias = 0.003f;
    kalman->R_measure = 0.03f;
}


float Kalman_Update(Kalman_t *kalman,
                    float newAngle,
                    float newRate,
                    float dt)
{
    float S;
    float K0;
    float K1;
    float y;

    float P00_temp;
    float P01_temp;

    /* Prediction */
    kalman->rate = newRate - kalman->bias;

    kalman->angle += dt * kalman->rate;

    kalman->P[0][0] +=
        dt * (dt * kalman->P[1][1]
        - kalman->P[0][1]
        - kalman->P[1][0]
        + kalman->Q_angle);

    kalman->P[0][1] -= dt * kalman->P[1][1];
    kalman->P[1][0] -= dt * kalman->P[1][1];
    kalman->P[1][1] += kalman->Q_bias * dt;


    /* Measurement update */
    S = kalman->P[0][0] + kalman->R_measure;

    K0 = kalman->P[0][0] / S;
    K1 = kalman->P[1][0] / S;

    y = newAngle - kalman->angle;

    kalman->angle += K0 * y;
    kalman->bias += K1 * y;


    /* Covariance update */
    P00_temp = kalman->P[0][0];
    P01_temp = kalman->P[0][1];

    kalman->P[0][0] -= K0 * P00_temp;
    kalman->P[0][1] -= K0 * P01_temp;

    kalman->P[1][0] -= K1 * P00_temp;
    kalman->P[1][1] -= K1 * P01_temp;

    return kalman->angle;
}