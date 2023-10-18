#ifndef PITCHER_H
#define PITCHER_H

#include <stdlib.h>
#include "baseball_objects.h"

void pitcher_init(tinygl_point_t *pitcher);

void pitcher_move_right(tinygl_point_t *pitcher);

void pitcher_move_left(tinygl_point_t *pitcher);

void pitcher_power_bar_init(power_bar_t *power_bar);

void pitcher_power_bar_update(power_bar_t *power_bar);

void fielder_init(tinygl_point_t *fielder, tinygl_point_t *hit_ball);

void fielder_move_up(tinygl_point_t *fielder);

void fielder_move_down(tinygl_point_t *fielder);

void fielder_move_right(tinygl_point_t *fielder);

void fielder_move_left(tinygl_point_t *fielder);

#endif