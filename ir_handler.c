#include "ir_handler.h"

bool ball_packet_valid_p(uint8_t ball_packet) 
{
    return !(ball_packet > 38 || ball_packet == 7 || ball_packet == 15 || ball_packet == 23 || ball_packet == 31);
}