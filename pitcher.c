#include "pitcher.h"

void pitcher_init(tinygl_point_t *pitcher) 
{
    pitcher->x = LEDMAT_COLS_NUM / 2;
    pitcher->y = LEDMAT_ROWS_NUM / 2;
}

void pitcher_move_right(tinygl_point_t *pitcher)
{
    if (pitcher->x < LEDMAT_COLS_NUM - 1) pitcher->x++;
}

void pitcher_move_left(tinygl_point_t *pitcher)
{
    if (pitcher->x > 0) pitcher->x--;
}

void pitcher_power_bar_init(power_bar_t *power_bar)
{
    power_bar->power = 0;
    power_bar->increment_value = 1;
}

void pitcher_power_bar_update(power_bar_t *power_bar)
{
    if (power_bar->power == LEDMAT_ROWS_NUM - 1)
        power_bar->increment_value = -1;

    if (power_bar->power == 0)
        power_bar->increment_value = 1;

    power_bar->power += power_bar->increment_value;
}

void fielder_init(tinygl_point_t *fielder, tinygl_point_t *hit_ball) 
{
    uint8_t random_corner = rand() % 4;
    fielder->x = (LEDMAT_COLS_NUM - 1) * (random_corner >> 1);
    fielder->y = (LEDMAT_ROWS_NUM - 1) * (random_corner & 1);
    hit_ball->x = (LEDMAT_COLS_NUM - 1) * !(random_corner >> 1);
    hit_ball->y = (LEDMAT_ROWS_NUM - 1) * !(random_corner & 1);
}

void fielder_move_up(tinygl_point_t *fielder) 
{
    if (fielder->y > 0) fielder->y--;
}

void fielder_move_down(tinygl_point_t *fielder)
{
    if (fielder->y < LEDMAT_ROWS_NUM - 1) fielder->y++;   
}

void fielder_move_right(tinygl_point_t *fielder)
{
    if (fielder->x < LEDMAT_COLS_NUM - 1) fielder->x++;
}

void fielder_move_left(tinygl_point_t *fielder)
{
    if (fielder->x > 0) fielder->x--;
}