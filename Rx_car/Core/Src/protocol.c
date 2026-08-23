#include "protocol.h"

#include <stdio.h>


void ProtocolRx_Init(ProtocolRx_t *protocol)
{
    if (protocol == NULL)
    {
        return;
    }

    protocol->index = 0;
}


bool ProtocolRx_PushByte(ProtocolRx_t *protocol,
                         uint8_t byte,
                         int16_t *forward,
                         int16_t *turn)
{
    if (protocol == NULL ||
        forward == NULL ||
        turn == NULL)
    {
        return false;
    }

    /* End of packet */
    if (byte == '\n')
    {
        protocol->buffer[protocol->index] = '\0';

        int f;
        int t;

        int result = sscanf(protocol->buffer,
                            "F:%d T:%d",
                            &f,
                            &t);

        protocol->index = 0;

        if (result == 2)
        {
            *forward = (int16_t)f;
            *turn = (int16_t)t;

            return true;
        }

        return false;
    }


    /* Ignore carriage return */
    if (byte == '\r')
    {
        return false;
    }


    /* Store incoming byte */
    if (protocol->index < PROTOCOL_BUFFER_SIZE - 1)
    {
        protocol->buffer[protocol->index++] = (char)byte;
    }
    else
    {
        /* Packet too long -> discard */
        protocol->index = 0;
    }

    return false;
}