#include "protocol.h"

#include <stdio.h>


uint16_t Protocol_Encode(int16_t forward,
                         int16_t turn,
                         char *buffer,
                         uint16_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0)
    {
        return 0;
    }

    int length = snprintf(buffer,
                          buffer_size,
                          "F:%d T:%d\n",
                          forward,
                          turn);

    if (length < 0 || length >= buffer_size)
    {
        return 0;
    }

    return (uint16_t)length;
}