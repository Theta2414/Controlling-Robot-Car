#include "control.h"

#define CONTROL_DEADZONE_DEG   5.0f
#define CONTROL_MAX_ANGLE_DEG  45.0f
#define CONTROL_MAX_OUTPUT     100


static int16_t angle_to_control(float angle)
{
    /* Deadzone around neutral */
    if (angle > -CONTROL_DEADZONE_DEG &&
        angle <  CONTROL_DEADZONE_DEG)
    {
        return 0;
    }

    /* Saturation */
    if (angle > CONTROL_MAX_ANGLE_DEG)
    {
        angle = CONTROL_MAX_ANGLE_DEG;
    }

    if (angle < -CONTROL_MAX_ANGLE_DEG)
    {
        angle = -CONTROL_MAX_ANGLE_DEG;
    }

    /* -45 ... +45 deg
     *       ↓
     * -100 ... +100
     */
    return (int16_t)(
        angle * CONTROL_MAX_OUTPUT
        / CONTROL_MAX_ANGLE_DEG
    );
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