#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include "system.h"

typedef enum {
    START_GAME = 'G',
    BALL_HIT = 'O',
    BALL_MISSED = 'X',
    BALL_FIELDED = 'F',
    STRIKED_OUT = 'S',
    END_GAME = 'E',
    YOU_WIN = 'W',
    YOU_LOSE = 'L'
} ir_message_t;

bool ball_packet_valid_p(uint8_t ball_packet);

#endif