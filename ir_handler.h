/** @file ir_handler.h
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief Abstracts IR communications
*/

#ifndef IR_HANDLER_H
#define IR_HANDLER_H

#include "system.h"

/**
 * Enum for the different messages transmitted over IR, to make it not just
 * characters
*/
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

/**
 * Returns true if ball_packet is one of the possible expected values
*/
bool ball_packet_valid_p(uint8_t ball_packet);

#endif