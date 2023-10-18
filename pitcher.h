/** @file pitcher.h
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief This module abstracts a lot of functionality regarding the "pitching team",
 *  including the fielder, and the power bar.
*/

#ifndef PITCHER_H
#define PITCHER_H

#include <stdlib.h>
#include "baseball_objects.h"

/**
 * Initialises the pitcher to be in the centre of the screen
*/
void pitcher_init(tinygl_point_t *pitcher);

/**
 * Move the pitcher one space to the right
*/
void pitcher_move_right(tinygl_point_t *pitcher);

/**
 * Move the pitcher one space to the left
*/
void pitcher_move_left(tinygl_point_t *pitcher);

/**
 * Initialise the power bar to be at 0 and increasing.
*/
void pitcher_power_bar_init(power_bar_t *power_bar);

/**
 * Increment the power bar until max, then decrement until min, and repeat
*/
void pitcher_power_bar_update(power_bar_t *power_bar);

/**
 * Initialise the fielder to be in a random corner, and for the hit ball
 * to be in the opposite corner.
*/
void fielder_init(tinygl_point_t *fielder, tinygl_point_t *hit_ball);

/**
 * Move the fielder up one space
*/
void fielder_move_up(tinygl_point_t *fielder);

/**
 * Move the fielder down one space
*/
void fielder_move_down(tinygl_point_t *fielder);

/**
 * Move the fielder right one space
*/
void fielder_move_right(tinygl_point_t *fielder);

/**
 * Move the fielder left one space
*/
void fielder_move_left(tinygl_point_t *fielder);

#endif