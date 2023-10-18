/** @file baseball_objects.h
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief This module contains the struct definitions of the moving parts of this baseball game
*/

#ifndef BASEBALL_OBJECTS_H
#define BASEBALL_OBJECTS_H

#include "system.h"
#include "tinygl.h"

#define BATTER_Y 5
#define BATTER_EXTRA_WIDTH 1
#define MAX_STRIKES 3
#define NUM_BASES 4

/**
 * Array of points representing the four bases - the order is important
*/
extern const tinygl_point_t BASES[];

/**
 * Quickly oscillating power meter for the pitcher to 'select' how fast 
 * the pitch will be
*/
typedef struct {
    uint8_t power;
    int8_t increment_value;
} power_bar_t;

/**
 * Ball as it appears on the batter's display. A higher move_rate means 
 * a harder to hit ball
*/
typedef struct {
    tinygl_point_t pos;
    uint8_t move_rate;
} ball_t;

/**
 * The batter as they run around the bases.
*/
typedef struct {
    tinygl_point_t pos;
    uint8_t base_num;
} runner_t;

/**
 * The batter as they are swinging agains the pitch
*/
typedef struct {
    tinygl_point_t pos;
    uint8_t extra_width;
} batter_t;

/**
 * Returns true if the given points are at the same position
*/
bool point_equals_p(tinygl_point_t point1, tinygl_point_t point2);

/**
 * Returns true if the batter swings successfully at the thrown ball
*/
bool ball_hit_p(ball_t pitched_ball, batter_t batter);

#endif