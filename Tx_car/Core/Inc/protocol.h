#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_BUFFER_SIZE 32

uint16_t Protocol_Encode(int16_t forward,
                         int16_t turn,
                         char *buffer,
                         uint16_t buffer_size);

#endif /* PROTOCOL_H */