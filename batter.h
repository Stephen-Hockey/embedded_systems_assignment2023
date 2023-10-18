/** @file batter.h
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief This file abstracts a lot of functionality regarding the "batting team",
 *  including the base runner.
*/

#ifndef BATTER_H
#define BATTER_H

#include "baseball_objects.h"

/**
 * Initialises the batter to be in the centre column, and one space in from the furthest row from the pitcher,
 * and to be 2 pixels long (1 extra pixel of width)
*/
void batter_init(batter_t *batter);

/**
 * Moves the batter left one space
*/
void batter_move_left(batter_t *batter);

/**
 * Moves the batter right one space
*/
void batter_move_right(batter_t *batter);

/**
 * Initializes the runner to be at the base he touched last
*/
void runner_init(runner_t *runner);

/**
 * Moves the runner up one space
*/
void runner_move_up(runner_t *runner);

/**
 * Moves the runner down one space
*/
void runner_move_down(runner_t *runner);

/**
 * Moves the runner right one space
*/
void runner_move_right(runner_t *runner);

/**
 * Moves the runner left one space
*/
void runner_move_left(runner_t *runner);

#endif