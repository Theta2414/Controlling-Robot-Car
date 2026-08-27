#include "control.h"

#include <math.h>

#define CONTROL_DEADZONE_DEG   8.0f
#define CONTROL_MAX_ANGLE_DEG  60.0f
#define CONTROL_MAX_OUTPUT     80.0f

static int16_t angle_to_control(float angle)
{
    float abs_angle = fabsf(angle);

    /* Neutral zone */
    if (abs_angle <= CONTROL_DEADZONE_DEG)
    {
        return 0;
    }

    /* Saturation */
    if (abs_angle >= CONTROL_MAX_ANGLE_DEG)
    {
        return (angle > 0.0f)
             ? CONTROL_MAX_OUTPUT
             : -CONTROL_MAX_OUTPUT;
    }

    /*
     * Normalize:
     * 8 deg  -> 0
     * 60 deg -> 1
     */
    float normalized =
        (abs_angle - CONTROL_DEADZONE_DEG) /
        (CONTROL_MAX_ANGLE_DEG - CONTROL_DEADZONE_DEG);

    /*
     * Expo response:
     * less sensitive near center,
     * stronger response near max tilt.
     */
    normalized = normalized * normalized;

    int16_t output =
        (int16_t)(normalized * CONTROL_MAX_OUTPUT);

    return (angle > 0.0f) ? output : -output;
}

void Control_Update(Control_t *control,
                    float pitch_deg,
                    float roll_deg)
{
    if (control == 0)
    {
        return;
    }

    control->forward =
        angle_to_control(pitch_deg);

    control->turn =
        angle_to_control(roll_deg);
}