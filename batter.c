/** @file batter.c
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief This file abstracts a lot of functionality regarding the "batting team",
 *  including the base runner.
*/

#include "batter.h"

/**
 * Initialises the batter to be in the centre column, and one space in from the furthest row from the pitcher,
 * and to be 2 pixels long (1 extra pixel of width)
*/
void batter_init(batter_t *batter)
{
    batter->pos.x = LEDMAT_COLS_NUM / 2;
    batter->pos.y = BATTER_Y;
    batter->extra_width = BATTER_EXTRA_WIDTH;}

/**
 * Moves the batter left one space
*/
void batter_move_left(batter_t *batter) 
{
    if (batter->pos.x > 0) batter->pos.x--;
}

/**
 * Moves the batter right one space
*/
void batter_move_right(batter_t *batter)
{
    if (batter->pos.x < LEDMAT_COLS_NUM - 1 - batter->extra_width) batter->pos.x++;
}

/**
 * Initializes the runner to be at the base he touched last
*/
void runner_init(runner_t *runner)
{
    runner->pos.x = BASES[runner->base_num].x;
    runner->pos.y = BASES[runner->base_num].y;
}

/**
 * Moves the runner up one space
*/
void runner_move_up(runner_t *runner)
{
    if (runner->pos.y > 0) runner->pos.y--;
}

/**
 * Moves the runner down one space
*/
void runner_move_down(runner_t *runner)
{
    if (runner->pos.y < LEDMAT_ROWS_NUM - 1) runner->pos.y++;
}

/**
 * Moves the runner right one space
*/
void runner_move_right(runner_t *runner)
{
    if (runner->pos.x < LEDMAT_COLS_NUM - 1) runner->pos.x++;
}

/**
 * Moves the runner left one space
*/
void runner_move_left(runner_t *runner)
{
    if (runner->pos.x > 0) runner->pos.x--;
}