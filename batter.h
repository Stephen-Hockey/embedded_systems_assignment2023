#ifndef BATTER_H
#define BATTER_H

#include "baseball_objects.h"

void batter_init(batter_t *batter);

void batter_move_left(batter_t *batter);

void batter_move_right(batter_t *batter);

void runner_init(runner_t *runner);

void runner_move_up(runner_t *runner);

void runner_move_down(runner_t *runner);

void runner_move_right(runner_t *runner);

void runner_move_left(runner_t *runner);

#endif