#include "baseball_objects.h"

const tinygl_point_t BASES[] = {
    {.x = 3, .y = 4},
    {.x = 3, .y = 2},
    {.x = 1, .y = 2},
    {.x = 1, .y = 4}
};

/**
 * Returns true if the given points are at the same position
 * @param point1 a point
 * @param point2 another point
*/
bool point_equals(tinygl_point_t point1, tinygl_point_t point2) 
{
    return point1.x == point2.x && point1.y == point2.y;
}

/**
 * Returns true if the batter's swing successfully hit the thrown ball
*/
bool ball_hit_p(ball_t pitched_ball, batter_t batter) 
{
    bool good_timing = pitched_ball.pos.y >= (BATTER_Y - 1) && pitched_ball.pos.y <= (BATTER_Y + 1);
    bool good_positioning = batter.pos.x >= pitched_ball.pos.x - 1 && batter.pos.x <= pitched_ball.pos.x;
    return good_timing && good_positioning;
}