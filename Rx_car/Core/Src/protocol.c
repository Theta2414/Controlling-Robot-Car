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


static bool parse_single_char(char c, int16_t *forward, int16_t *turn)
{
    switch (c)
    {
        case 'F': /* Tiến thẳng */
            *forward = 80;
            *turn = 0;
            return true;
        case 'B': /* Lùi thẳng */
            *forward = -80;
            *turn = 0;
            return true;
        case 'L': /* Xoay trái tại chỗ */
            *forward = 0;
            *turn = -80;
            return true;
        case 'R': /* Xoay phải tại chỗ */
            *forward = 0;
            *turn = 80;
            return true;
        case 'G': /* Tiến - Trái (Forward Left) */
            *forward = 80;
            *turn = -48;
            return true;
        case 'I': /* Tiến - Phải (Forward Right) */
            *forward = 80;
            *turn = 48;
            return true;
        case 'H': /* Lùi - Trái (Backward Left) */
            *forward = -80;
            *turn = -48;
            return true;
        case 'J': /* Lùi - Phải (Backward Right) */
            *forward = -80;
            *turn = 48;
            return true;
        case 'S': /* Dừng */
        case 'D':
        case '0':
            *forward = 0;
            *turn = 0;
            return true;
        default:
            return false;
    }
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

    /* End of packet delimiter */
    if (byte == '\n')
    {
        protocol->buffer[protocol->index] = '\0';
        uint16_t len = protocol->index;
        protocol->index = 0;

        if (len == 1)
        {
            return parse_single_char(protocol->buffer[0], forward, turn);
        }

        int f;
        int t;

        int result = sscanf(protocol->buffer,
                            "F:%d T:%d",
                            &f,
                            &t);

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

    /* Immediate single-byte commands (Bluetooth RC controller stream without newline) */
    if (protocol->index == 0)
    {
        if (byte == 'G' || byte == 'I' || byte == 'H' || byte == 'J' ||
            byte == 'S' || byte == 'D' || byte == 'L' || byte == 'R' ||
            byte == 'B' || byte == '0')
        {
            return parse_single_char((char)byte, forward, turn);
        }
    }
    else if (protocol->index == 1 && protocol->buffer[0] == 'F' && byte != ':')
    {
        /* Previous byte was standalone 'F' command (not start of "F:") */
        char prev = protocol->buffer[0];
        protocol->index = 0;
        if (byte != 'F')
        {
            protocol->buffer[protocol->index++] = (char)byte;
        }
        return parse_single_char(prev, forward, turn);
    }

    /* Store incoming byte for "F:%d T:%d" packet */
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