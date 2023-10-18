#ifndef BASEBALL_OBJECTS_H
#define BASEBALL_OBJECTS_H

#include "system.h"
#include "tinygl.h"

#define BATTER_Y 5
#define BATTER_EXTRA_WIDTH 1
#define MAX_STRIKES 3
#define NUM_BASES 4

extern const tinygl_point_t BASES[];

typedef struct {
    uint8_t power;
    int8_t increment_value;
} power_bar_t;

typedef struct {
    tinygl_point_t pos;
    uint8_t move_rate;
} ball_t;

typedef struct {
    tinygl_point_t pos;
    uint8_t base_num;
} runner_t;

typedef struct {
    tinygl_point_t pos;
    uint8_t extra_width;
} batter_t;

bool point_equals(tinygl_point_t point1, tinygl_point_t point2);

bool ball_hit_p(ball_t pitched_ball, batter_t batter);

#endif