#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#define PROTOCOL_BUFFER_SIZE 32

typedef struct
{
    char buffer[PROTOCOL_BUFFER_SIZE];
    uint16_t index;

} ProtocolRx_t;


void ProtocolRx_Init(ProtocolRx_t *protocol);

bool ProtocolRx_PushByte(ProtocolRx_t *protocol,
                         uint8_t byte,
                         int16_t *forward,
                         int16_t *turn);

#endif /* PROTOCOL_H */