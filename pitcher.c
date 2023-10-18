/** @file pitcher.c
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief This file abstracts a lot of functionality regarding the "pitching team",
 *  including the fielder, and the power bar.
*/

#include "pitcher.h"

/**
 * Initialises the pitcher to be in the centre of the screen
*/
void pitcher_init(tinygl_point_t *pitcher) 
{
    pitcher->x = LEDMAT_COLS_NUM / 2;
    pitcher->y = LEDMAT_ROWS_NUM / 2;
}

/**
 * Move the pitcher one space to the right
*/
void pitcher_move_right(tinygl_point_t *pitcher)
{
    if (pitcher->x < LEDMAT_COLS_NUM - 1) pitcher->x++;
}

/**
 * Move the pitcher one space to the left
*/
void pitcher_move_left(tinygl_point_t *pitcher)
{
    if (pitcher->x > 0) pitcher->x--;
}

/**
 * Initialise the power bar to be at 0 and increasing.
*/
void pitcher_power_bar_init(power_bar_t *power_bar)
{
    power_bar->power = 0;
    power_bar->increment_value = 1;
}

/**
 * Increment the power bar until max, then decrement until min, and repeat
*/
void pitcher_power_bar_update(power_bar_t *power_bar)
{
    if (power_bar->power == LEDMAT_ROWS_NUM - 1)
        power_bar->increment_value = -1;

    if (power_bar->power == 0)
        power_bar->increment_value = 1;

    power_bar->power += power_bar->increment_value;
}

/**
 * Initialise the fielder to be in a random corner, and for the hit ball
 * to be in the opposite corner.
*/
void fielder_init(tinygl_point_t *fielder, tinygl_point_t *hit_ball) 
{
    uint8_t random_corner = rand() % 4;
    fielder->x = (LEDMAT_COLS_NUM - 1) * (random_corner >> 1);
    fielder->y = (LEDMAT_ROWS_NUM - 1) * (random_corner & 1);
    hit_ball->x = (LEDMAT_COLS_NUM - 1) * !(random_corner >> 1);
    hit_ball->y = (LEDMAT_ROWS_NUM - 1) * !(random_corner & 1);
}

/**
 * Move the fielder up one space
*/
void fielder_move_up(tinygl_point_t *fielder) 
{
    if (fielder->y > 0) fielder->y--;
}

/**
 * Move the fielder down one space
*/
void fielder_move_down(tinygl_point_t *fielder)
{
    if (fielder->y < LEDMAT_ROWS_NUM - 1) fielder->y++;   
}

/**
 * Move the fielder right one space
*/
void fielder_move_right(tinygl_point_t *fielder)
{
    if (fielder->x < LEDMAT_COLS_NUM - 1) fielder->x++;
}

/**
 * Move the fielder left one space
*/
void fielder_move_left(tinygl_point_t *fielder)
{
    if (fielder->x > 0) fielder->x--;
}