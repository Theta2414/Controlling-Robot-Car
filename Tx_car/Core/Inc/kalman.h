#ifndef INC_KALMAN_H_
#define INC_KALMAN_H_

typedef struct
{
    float angle;
    float bias;
    float rate;

    float P[2][2];

    float Q_angle;
    float Q_bias;
    float R_measure;

} Kalman_t;

void Kalman_Init(Kalman_t *kalman);

float Kalman_Update(Kalman_t *kalman,
                    float newAngle,
                    float newRate,
                    float dt);

#endif /* INC_KALMAN_H_ */