#include "batter.h"

void batter_init(batter_t *batter)
{
    batter->pos.x = LEDMAT_COLS_NUM / 2;
    batter->pos.y = BATTER_Y;
    batter->extra_width = BATTER_EXTRA_WIDTH;}

void batter_move_left(batter_t *batter) 
{
    if (batter->pos.x > 0) batter->pos.x--;
}

void batter_move_right(batter_t *batter)
{
    if (batter->pos.x < LEDMAT_COLS_NUM - 1 - batter->extra_width) batter->pos.x++;
}

void runner_init(runner_t *runner)
{
    runner->pos.x = BASES[runner->base_num].x;
    runner->pos.y = BASES[runner->base_num].y;
}

void runner_move_up(runner_t *runner)
{
    if (runner->pos.y > 0) runner->pos.y--;
}

void runner_move_down(runner_t *runner)
{
    if (runner->pos.y < LEDMAT_ROWS_NUM - 1) runner->pos.y++;
}

void runner_move_right(runner_t *runner)
{
    if (runner->pos.x < LEDMAT_COLS_NUM - 1) runner->pos.x++;
}

void runner_move_left(runner_t *runner)
{
    if (runner->pos.x > 0) runner->pos.x--;
}