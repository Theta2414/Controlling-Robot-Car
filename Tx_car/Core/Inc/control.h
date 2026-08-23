#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

typedef struct
{
    int16_t forward;
    int16_t turn;

} Control_t;


void Control_Update(Control_t *control,
                    float pitch_deg,
                    float roll_deg);

#endif /* CONTROL_H */