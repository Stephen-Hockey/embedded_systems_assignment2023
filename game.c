#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "pio.h"
#include "timer.h"
#include "pacer.h"
#include "button.h"
#include "navswitch.h"
#include "led.h"
#include "ledmat.h"
#include "display.h"
#include "tinygl.h"
#include "font.h"
#include "../fonts/font5x7_1.h"
#include "ir_uart.h"

#define PACER_RATE 200
#define NAVSWITCH_CHECK_RATE 200
#define COLLISION_CHECK_RATE 200
#define DRAW_HARD_RATE 200
#define DRAW_LIGHT_RATE 50
#define POWER_BAR_UPDATE_RATE 20
#define IR_CHECK_RATE 200

#define CNTR_COL 2
#define CNTR_ROW 3
#define BATTER_Y 5
#define BATTER_EXTRA_WIDTH 1
#define MAX_STRIKES 3

typedef enum {
    GAME_LAUNCHED,          /* When a game is first started, players need to select the batter/pitcher */
    PITCHER_CHOOSE,         /* Pitcher is choosing what lane to throw the ball down */
    PITCHER_TIMING,         /* Pitcher is timing the power of his shot */
    PITCHER_BALL_THROWN,    /* Pitcher has thrown the ball and is waiting for batter's response */
    PITCHER_FIELDING,       /* Ball has been hit and a fielder has to pick it up */
    BATTER_IDLE,            /* Batter is waiting for the ball to be thrown */
    BATTER_BALL_THROWN,     /* Ball comes towards batter for them to swing at */
    BATTER_RUNNING,         /* Ball has been hit and batter can run around the bases */
    GAME_END                /* Both players have had a chance at both roles and the game is over */
} game_state_t;

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

static game_state_t game_state = GAME_LAUNCHED;

static tinygl_point_t pitcher = {.x = CNTR_COL, .y = CNTR_ROW}; 
static batter_t batter = {.pos.x = CNTR_COL, .pos.y = BATTER_Y, .extra_width = BATTER_EXTRA_WIDTH};
static const tinygl_point_t FIRST_BASE = {.x = CNTR_COL + 1, .y = CNTR_ROW + 1};
static const tinygl_point_t SECOND_BASE = {.x = CNTR_COL + 1, .y = CNTR_ROW - 1};
static const tinygl_point_t THIRD_BASE = {.x = CNTR_COL - 1, .y = CNTR_ROW - 1};
static const tinygl_point_t HOME_BASE = {.x = CNTR_COL - 1, .y = CNTR_ROW + 1};
static tinygl_point_t fielder;
static tinygl_point_t landed_ball;
static ball_t thrown_ball;
static runner_t runner;
static power_bar_t power_bar;

static uint16_t homeruns;
static uint8_t strikes;

static uint8_t chosen_pitch_col;
static uint8_t chosen_pitch_power;

/**
 * Returns true if the batter's swing successfully hit the thrown ball
*/
bool ball_hit_p() 
{
    bool good_timing = thrown_ball.pos.y >= (BATTER_Y - 1) && thrown_ball.pos.y <= (BATTER_Y + 1);
    bool good_positioning = batter.pos.x >= thrown_ball.pos.x - 1 && batter.pos.x <= thrown_ball.pos.x;
    return good_timing && good_positioning;
}

/**
 * Returns true if the given points are at the same position
 * @param point1 a point
 * @param point2 another point
*/
bool point_equals(tinygl_point_t point1, tinygl_point_t point2) 
{
    return point1.x == point2.x && point1.y == point2.y;
}

void navswitch_north_pushed()
{
    switch (game_state) {
        case PITCHER_FIELDING:
            if (fielder.y > 0)
                fielder.y--;
            break;
        case BATTER_BALL_THROWN:
            if (ball_hit_p()) {
                runner.pos.x = 1;
                runner.pos.y = 4;
                runner.base_num = 0;
                game_state = BATTER_RUNNING;
                ir_uart_putc('O');
            }
            break;        
        case BATTER_RUNNING:
            if (runner.pos.y > 0)
                runner.pos.y--;
            break;
    }
}

void navswitch_south_pushed()
{
    switch (game_state) {
        case PITCHER_FIELDING:
            if (fielder.y < LEDMAT_ROWS_NUM - 1)
                fielder.y++;        
            break;
        case BATTER_RUNNING:
            if (runner.pos.y < LEDMAT_ROWS_NUM - 1)
                runner.pos.y++;
            break;
    }
}

void navswitch_east_pushed()
{
    switch (game_state) {
        case PITCHER_CHOOSE:
            if (pitcher.x < LEDMAT_COLS_NUM - 1)
                pitcher.x++;
            break;    
        case PITCHER_FIELDING:
            if (fielder.x < LEDMAT_COLS_NUM - 1)
                fielder.x++;
            break;
        case BATTER_IDLE:
            if (batter.pos.x < LEDMAT_COLS_NUM - 1 - batter.extra_width)
                batter.pos.x++;
            break;    
        case BATTER_BALL_THROWN:
            if (batter.pos.x < LEDMAT_COLS_NUM - 1 - batter.extra_width)
                batter.pos.x++;
            break;   
        case BATTER_RUNNING:
            if (runner.pos.x < LEDMAT_COLS_NUM - 1)
                runner.pos.x++;
            break;
    }
}

void navswitch_west_pushed()
{   
    switch (game_state) {
        case PITCHER_CHOOSE:
            if (pitcher.x > 0)
                pitcher.x--;
            break;  
        case PITCHER_FIELDING:
            if (fielder.x > 0)
                fielder.x--;
            break;
        case BATTER_IDLE:
            if (batter.pos.x > 0)
                batter.pos.x--;
            break;    
        case BATTER_BALL_THROWN:
            if (batter.pos.x > 0)
                batter.pos.x--;
            break;        
        case BATTER_RUNNING:
            if (runner.pos.x > 0)
                runner.pos.x--;
            break;
    }
}

void navswitch_push_pushed()
{
    uint8_t ball_packet; 
    switch (game_state) {
        case GAME_LAUNCHED:
            ir_uart_putc('A'); // the character is irrelevant but can use for checking
            game_state = BATTER_IDLE;        
            break;
        case PITCHER_CHOOSE:
            chosen_pitch_col = pitcher.x;
            power_bar.power = 0;
            power_bar.increment_value = 1;
            game_state = PITCHER_TIMING;
            break;    
        case PITCHER_TIMING:        
            chosen_pitch_power = power_bar.power;
            ball_packet = (chosen_pitch_col << 3) | chosen_pitch_power;
            ir_uart_putc(ball_packet);
            game_state = PITCHER_BALL_THROWN;
            break;
    }
}

/**
 * Checks whether there was a navswitch push event at NAVSWITCH_CHECK_RATE Hz
 * and calls the appropriate function
*/
void check_navswitch() {
    navswitch_update ();
    if (navswitch_push_event_p (NAVSWITCH_NORTH))
        navswitch_north_pushed();
    if (navswitch_push_event_p (NAVSWITCH_SOUTH))
        navswitch_south_pushed();
    if (navswitch_push_event_p (NAVSWITCH_EAST))
        navswitch_east_pushed();
    if (navswitch_push_event_p (NAVSWITCH_WEST))
        navswitch_west_pushed();
    if (navswitch_push_event_p (NAVSWITCH_PUSH))
        navswitch_push_pushed();    
}

bool ball_packet_valid_p(uint8_t ball_packet) {
    return !(ball_packet > 38 || ball_packet == 7 || ball_packet == 15 || ball_packet == 23 || ball_packet == 31);
}

void check_collisions()
{
    switch (game_state) {
        case PITCHER_FIELDING:
            if (point_equals(fielder, landed_ball)) {
                ir_uart_putc('F');
                pitcher.x = CNTR_COL;
                game_state = PITCHER_CHOOSE;
            }
            break;
        case BATTER_RUNNING:
            if (point_equals(runner.pos, FIRST_BASE) && runner.base_num == 0)
                runner.base_num = 1;
            if (point_equals(runner.pos, SECOND_BASE) && runner.base_num == 1)
                runner.base_num = 2;
            if (point_equals(runner.pos, THIRD_BASE) && runner.base_num == 2)
                runner.base_num = 3;
            if (point_equals(runner.pos, HOME_BASE) && runner.base_num == 3) {
                runner.base_num = 0;
                homeruns++;
            }
            break;
    }
}

void check_ir()
{
    uint8_t ball_packet;
    uint8_t hit_packet;
    uint8_t random_corner;

    switch (game_state) {
        case GAME_LAUNCHED:
            if (ir_uart_read_ready_p()) {
                if (ir_uart_getc() == 'A') {
                    game_state = PITCHER_CHOOSE;
                }            
            }
            break;
        case PITCHER_CHOOSE:
            if (ir_uart_read_ready_p()) {
                if (ir_uart_getc() == 'E') {
                    game_state = GAME_END;
                }            
            }
            break;   
        case PITCHER_BALL_THROWN:
            if (ir_uart_read_ready_p()) {
                hit_packet = ir_uart_getc();
                if (hit_packet == 'X') {
                    pitcher.x = CNTR_COL;
                    game_state = PITCHER_CHOOSE;
                } else if (hit_packet == 'O') {
                    random_corner = rand() % 4;

                    fielder.x = (LEDMAT_COLS_NUM - 1) * (random_corner >> 1);
                    fielder.y = (LEDMAT_ROWS_NUM - 1) * (random_corner & 1);
                    landed_ball.x = (LEDMAT_COLS_NUM - 1) * !(random_corner >> 1);
                    landed_ball.y = (LEDMAT_ROWS_NUM - 1) * !(random_corner & 1);     

                    game_state = PITCHER_FIELDING;
                } else if (hit_packet == 'S') {
                    if (strikes == 0) {
                        game_state = BATTER_IDLE;
                    } else {
                        game_state = GAME_END;
                        ir_uart_putc('E');
                    }
                }
            }
            break;
        case BATTER_IDLE:
            if (ir_uart_read_ready_p()) {
                ball_packet = ir_uart_getc();
                if (ball_packet_valid_p(ball_packet)) {
                    chosen_pitch_col = ball_packet >> 0x03;
                    chosen_pitch_power = ball_packet & 0x07;
                    thrown_ball.pos.x = LEDMAT_COLS_NUM - 1 - chosen_pitch_col;
                    thrown_ball.pos.y = 0;
                    switch (chosen_pitch_power)
                    {
                    case 0:
                        thrown_ball.move_rate = 2;
                        break; 
                    case 1:
                    case 2:
                    case 3:
                        thrown_ball.move_rate = 5;
                        break;    
                    case 4:
                        thrown_ball.move_rate = 10;
                    case 5:
                        thrown_ball.move_rate = 20;
                        break;    
                    case 6:
                        thrown_ball.move_rate = 40;
                        break;        
                    default:
                        break;
                    }
                    game_state = BATTER_BALL_THROWN;
                }            
            }
            break;     
        case BATTER_RUNNING:
            if (ir_uart_read_ready_p()) {
                if (ir_uart_getc() == 'F') {
                    batter.pos.x = CNTR_COL;
                    game_state = BATTER_IDLE;
                }           
            }
            break;
    }
}

void update_power_bar() 
{   
    if (power_bar.power == LEDMAT_ROWS_NUM - 1)
        power_bar.increment_value = -1;

    if (power_bar.power == 0)
        power_bar.increment_value = 1;

    power_bar.power += power_bar.increment_value;
}

void update_thrown_ball()
{
    if (thrown_ball.pos.y < LEDMAT_ROWS_NUM) { // intentionally goes to 7
        thrown_ball.pos.y++;
    } else {
        strikes++;
        if (strikes == MAX_STRIKES) {
            game_state = PITCHER_CHOOSE;
            ir_uart_putc('S');
            return;
        }
        ir_uart_putc('X');
        batter.pos.x = CNTR_COL;
        game_state = BATTER_IDLE;
    }        
}

void draw_hard()
{   
    tinygl_point_t power_bar_left_point = {.x = 0, .y = LEDMAT_ROWS_NUM - 1};
    tinygl_point_t power_bar_right_point = {.x = LEDMAT_COLS_NUM - 1, .y = LEDMAT_ROWS_NUM - 1};
    tinygl_point_t batter_right_point = {.x = batter.pos.x + batter.extra_width, .y = batter.pos.y};

    switch (game_state) {
        case GAME_LAUNCHED:
            //TODO
            break;
        case PITCHER_CHOOSE:
            tinygl_draw_point(pitcher, 1);
            break;
        case PITCHER_TIMING:
            for (uint8_t i = 0; i <= power_bar.power; i++) {
                power_bar_left_point.y = LEDMAT_ROWS_NUM - 1 - i;
                power_bar_right_point.y = LEDMAT_ROWS_NUM - 1 - i;
                tinygl_draw_line(power_bar_left_point, power_bar_right_point, 1);            
            }
            break;
        case PITCHER_FIELDING:
            tinygl_draw_point(fielder, 1);
            break;
        case BATTER_IDLE:
            tinygl_draw_line(batter.pos, batter_right_point, 1);
            break;    
        case BATTER_BALL_THROWN:
            tinygl_draw_point(thrown_ball.pos, 1);
            tinygl_draw_line(batter.pos, batter_right_point, 1);
            break;        
        case BATTER_RUNNING:
            tinygl_draw_point(runner.pos, 1);
            break;
        case GAME_END:
            //TODO
            break;
    }
}

void draw_light()
{   
    switch (game_state) {
        case PITCHER_FIELDING:        
            tinygl_draw_point(landed_ball, 1);
            break;
        case BATTER_RUNNING:
            tinygl_draw_point(FIRST_BASE, 1);
            tinygl_draw_point(SECOND_BASE, 1);
            tinygl_draw_point(THIRD_BASE, 1);
            tinygl_draw_point(HOME_BASE, 1);
            break;
    }
}

int main (void)
{   
    // Initialisation
    system_init ();
    pacer_init(PACER_RATE);
    tinygl_init(PACER_RATE);
    navswitch_init();
    button_init();
    led_init();
    led_set(LED1, 0);
    ir_uart_init();

    // Declare tick counters
    uint8_t navswitch_check_ticks;
    uint8_t collision_check_ticks;
    uint8_t power_bar_update_ticks;
    uint8_t thrown_ball_ticks;
    uint8_t ir_ticks;
    uint8_t draw_hard_ticks;
    uint8_t draw_light_ticks;

    // Main game loop
    while (1) {
        pacer_wait();
        tinygl_clear();

        navswitch_check_ticks++;
        if (navswitch_check_ticks >= PACER_RATE/COLLISION_CHECK_RATE) {
            check_navswitch();
            navswitch_check_ticks = 0;
        }

        collision_check_ticks++;
        if (collision_check_ticks >= PACER_RATE/NAVSWITCH_CHECK_RATE) {
            check_collisions();
            collision_check_ticks = 0;
        }

        //TODO look towards putting this in a greater 'logic' function
        if (game_state == PITCHER_TIMING) {
            power_bar_update_ticks++;
            if (power_bar_update_ticks >= PACER_RATE/POWER_BAR_UPDATE_RATE) {
                update_power_bar();
                power_bar_update_ticks = 0;
            }
        }

        if (game_state == BATTER_BALL_THROWN) {
            thrown_ball_ticks++;
            if (thrown_ball_ticks >= PACER_RATE/thrown_ball.move_rate) {
                update_thrown_ball();
                thrown_ball_ticks = 0;
            }
        }

        ir_ticks++;
        if (ir_ticks >= PACER_RATE/IR_CHECK_RATE) {
            check_ir();
            ir_ticks = 0;
        }

        draw_hard_ticks++;
        if (draw_hard_ticks >= PACER_RATE/DRAW_HARD_RATE) {
            draw_hard();
            draw_hard_ticks = 0;
        }

        draw_light_ticks++;
        if (draw_light_ticks >= PACER_RATE/DRAW_LIGHT_RATE) {
            draw_light();
            draw_light_ticks = 0;
        }
        
        tinygl_update ();
    }
}