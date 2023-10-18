/** @file ir_handler.c
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief A small helper file to make transmitting/recieving IR a little easier.
*/

#include "ir_handler.h"

/**
 * Returns true if ball_packet is one of the possible expected values
*/
bool ball_packet_valid_p(uint8_t ball_packet) 
{
    return !(ball_packet > 38 || ball_packet == 7 || ball_packet == 15 || ball_packet == 23 || ball_packet == 31);
}