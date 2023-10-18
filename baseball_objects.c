/** @file baseball_objects.c
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief This module contains some helpful functions regarding the positions of the
 *  different objects present in the game, as well as a constant array of bases for the runner to run around.
*/

#include "baseball_objects.h"

/**
 * Array of points representing the four bases - the order is important
*/
const tinygl_point_t BASES[] = {
    {.x = 1, .y = 4},   // Home
    {.x = 3, .y = 4},   // First
    {.x = 3, .y = 2},   // Second
    {.x = 1, .y = 2}    // Third
    
};

/**
 * Returns true if the given points are at the same position
*/
bool point_equals_p(tinygl_point_t point1, tinygl_point_t point2) 
{
    return point1.x == point2.x && point1.y == point2.y;
}

/**
 * Returns true if the batter swings successfully at the thrown ball
*/
bool ball_hit_p(ball_t pitched_ball, batter_t batter) 
{
    bool good_timing = pitched_ball.pos.y >= (BATTER_Y - 1) && pitched_ball.pos.y <= (BATTER_Y + 1);
    bool good_positioning = batter.pos.x >= pitched_ball.pos.x - 1 && batter.pos.x <= pitched_ball.pos.x;
    return good_timing && good_positioning;
}