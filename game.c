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
#define DRAW_HARD_RATE 200
#define DRAW_LIGHT_RATE 50
#define POWER_BAR_UPDATE_RATE 10
#define IR_CHECK_RATE 200


typedef enum {
    GAME_LAUNCHED,          /* When a game is first started, roles need to be chosen */
    PITCHER_CHOOSE,         /* Pitcher is choosing what lane to throw the ball down */
    PITCHER_TIMING,         /* Pitcher is timing the power of his shot */
    PITCHER_BALL_THROWN,    /* Pitcher has thrown the ball and is waiting for batter's response */
    PITCHER_FIELDING,       
    BATTER_IDLE,            /* Batter is waiting for the ball to be thrown */
    BATTER_BALL_THROWN,     /* Ball comes towards batter for them to swing at */
    BATTER_RUNNING
    // TODO what happens after ball is hit/missed
} game_state_t;

/**
 * The pitcher, which is restricted to east-west movement, and represented by a pixel
 * 
*/
typedef struct {
    uint8_t col;
    const uint8_t row;
} pitcher_t;

typedef struct {
    uint8_t power;
    int8_t increment_value;
} power_bar_t;

/**
 * The batter, which is restricted to east-west movement, and represented by two pixels
*/
typedef struct {
    uint8_t col;
    const uint8_t row;
} batter_t;

typedef struct {
    uint8_t col;
    uint8_t row;
    uint8_t tick_rate;
} thrown_ball_t;

typedef struct {
    uint8_t col;
    uint8_t row;
} fielder_t;

typedef struct {
    uint8_t col;
    uint8_t row;
} landed_ball_t;

static game_state_t game_state = GAME_LAUNCHED; // Should be set to GAME_LAUNCHED initally

static pitcher_t pitcher = {.col = 2, .row = 3}; 
static power_bar_t power_bar = {.power = 0, .increment_value = 1};
static batter_t batter = {.col = 2, .row = 6};
static thrown_ball_t thrown_ball;
static fielder_t fielder;
static landed_ball_t landed_ball;
static uint16_t homeruns;
static uint8_t strikes;

// IR values
static uint8_t chosen_pitch_col;
static uint8_t chosen_pitch_power;
static const char LAUNCH_MSG[] = "BATTER UP?"; //TODO

bool ball_hit_p() {
    bool good_timing = thrown_ball.row >= 4 && thrown_ball.row <= 6;
    bool good_positioning = batter.col >= thrown_ball.col - 1 && batter.col <= thrown_ball.col;
    return good_timing && good_positioning;
}

void navswitch_north_pushed()
{
    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        break;    
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case PITCHER_FIELDING:
        break;
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
        // the swing
        if (ball_hit_p()) {
            homeruns++;
            game_state = BATTER_RUNNING;
            ir_uart_putc('O');
        }
        break;        
    default:
        break;
    }
}

void navswitch_south_pushed()
{
    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        break;    
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case PITCHER_FIELDING:
        break;
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
        break;        
    case BATTER_RUNNING:
        break;
    default:
        break;
    }
}

void navswitch_east_pushed()
{
    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        if (pitcher.col < LEDMAT_COLS_NUM - 1)
            pitcher.col++;
        break;    
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:    
        break;    
    case PITCHER_FIELDING:
        break;
    case BATTER_IDLE:
        if (batter.col < LEDMAT_COLS_NUM - 2)
            batter.col++;
        break;    
    case BATTER_BALL_THROWN:
        if (batter.col < LEDMAT_COLS_NUM - 2)
            batter.col++;
        break;   
    case BATTER_RUNNING:
        break;     
    default:
        break;
    }
}

void navswitch_west_pushed()
{   
    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        if (pitcher.col > 0)
            pitcher.col--;
        break;    
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case PITCHER_FIELDING:
        break;
    case BATTER_IDLE:
        if (batter.col > 0)
            batter.col--;
        break;    
    case BATTER_BALL_THROWN:
        if (batter.col > 0)
            batter.col--;
        break;        
    case BATTER_RUNNING:
        break;
    default:
        break;
    }
}

void navswitch_push_pushed()
{
    uint8_t ball_packet; 
    switch (game_state) {
    case GAME_LAUNCHED:
        ir_uart_putc('a'); // the character is irrelevant but can use for checking
        game_state = BATTER_IDLE;        
        break;
    case PITCHER_CHOOSE:
        chosen_pitch_col = pitcher.col;
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
    case PITCHER_BALL_THROWN:
        break;    
    case PITCHER_FIELDING:
        break;
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
        break;        
    case BATTER_RUNNING:
        break;
    default:
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

void check_ir()
{
    uint8_t ball_packet;
    uint8_t hit_packet;
    uint8_t random_corner;

    switch (game_state) {
    case GAME_LAUNCHED:
        if (ir_uart_read_ready_p()) {
            if (ir_uart_getc() == 'a') {
                game_state = PITCHER_CHOOSE;
            }            
        }
        break;
    case PITCHER_CHOOSE:
        break;    
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:
        if (ir_uart_read_ready_p()) {
            hit_packet = ir_uart_getc();
            if (hit_packet == 'X') {
                pitcher.col = 2;
                game_state = PITCHER_CHOOSE;
            } else if (hit_packet == 'O') {
                random_corner = rand() % 4;

                // fancy code because I didn't want to use another switch-case
                fielder.col = (LEDMAT_COLS_NUM - 1) * (random_corner >> 1);
                fielder.row = (LEDMAT_ROWS_NUM - 1) * (random_corner & 1);
                landed_ball.col = (LEDMAT_COLS_NUM - 1) * !(random_corner >> 1);
                landed_ball.row = (LEDMAT_ROWS_NUM - 1) * !(random_corner & 1);     

                game_state = PITCHER_FIELDING;
            }
        }
        break;    
    case PITCHER_FIELDING:
        break;
    case BATTER_IDLE:
        if (ir_uart_read_ready_p()) {
            ball_packet = ir_uart_getc();
            if (ball_packet_valid_p) {
                chosen_pitch_col = ball_packet >> 3;
                chosen_pitch_power = ball_packet & 0b111;
                thrown_ball.col = chosen_pitch_col;
                thrown_ball.row = 0;
                switch (chosen_pitch_power)
                {
                case 0:
                    thrown_ball.tick_rate = 2;
                    break; 
                case 1:
                case 2:
                case 3:
                    thrown_ball.tick_rate = 5;
                    break;    
                case 4:
                    thrown_ball.tick_rate = 10;
                case 5:
                    thrown_ball.tick_rate = 20;
                    break;    
                case 6:
                    thrown_ball.tick_rate = 40;
                    break;        
                default:
                    break;
                }
                game_state = BATTER_BALL_THROWN;
            }            
        }
        break;    
    case BATTER_BALL_THROWN:
        break;        
    case BATTER_RUNNING:
    
        break;
    default:
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
    if (thrown_ball.row < LEDMAT_ROWS_NUM) { // intentionally goes to 7
        thrown_ball.row++;
    } else {
        strikes++;
        ir_uart_putc('X');
        batter.col = 2;
        game_state = BATTER_IDLE;

    }
        
}

void draw_hard()
{   
    tinygl_point_t pitcher_point = {pitcher.col, pitcher.row};
    tinygl_point_t power_bar_left_point = {.x = 0};
    tinygl_point_t power_bar_right_point = {.x = LEDMAT_COLS_NUM - 1};
    tinygl_point_t batter_left_point = {batter.col, batter.row};
    tinygl_point_t batter_right_point = {batter.col + 1, batter.row};
    tinygl_point_t thrown_ball_point = {thrown_ball.col, thrown_ball.row};
    tinygl_point_t fielder_point = {fielder.col, fielder.row};
    // tinygl_point_t landed_ball_point = {landed_ball.col, landed_ball.row};


    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        tinygl_draw_point(pitcher_point, 1);  
        break;
    case PITCHER_TIMING:
        for (uint8_t i = 0; i <= power_bar.power; i++) {
            power_bar_left_point.y = LEDMAT_ROWS_NUM - 1 - i;
            power_bar_right_point.y = LEDMAT_ROWS_NUM - 1 - i;
            tinygl_draw_line(power_bar_left_point, power_bar_right_point, 1);            
        }
        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case PITCHER_FIELDING:
        led_set(LED1, 1);
        tinygl_draw_point(fielder_point, 1);
        break;
    case BATTER_IDLE:
        tinygl_draw_line(batter_left_point, batter_right_point, 1);
        break;    
    case BATTER_BALL_THROWN:
        tinygl_draw_point(thrown_ball_point, 1);
        tinygl_draw_line(batter_left_point, batter_right_point, 1);
        break;        
    case BATTER_RUNNING:
        led_set(LED1, 1);

        break;
    default:
        break;
    }
}

void draw_light()
{   
    tinygl_point_t landed_ball_point = {landed_ball.col, landed_ball.row};

    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        break;
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case PITCHER_FIELDING:        
        tinygl_draw_point(landed_ball_point, 1);
        break;
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
        break;        
    case BATTER_RUNNING:
        //TODO

        break;
    default:
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
        if (navswitch_check_ticks >= PACER_RATE/NAVSWITCH_CHECK_RATE) {
            check_navswitch();
            navswitch_check_ticks = 0;
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
            if (thrown_ball_ticks >= PACER_RATE/thrown_ball.tick_rate) {
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