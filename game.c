#include <stdio.h>
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

#include "ir_uart.h"

#define PACER_RATE 200
#define NAVSWITCH_CHECK_RATE 200
#define DRAW_RATE 200
#define POWER_BAR_UPDATE_RATE 2
#define IR_CHECK_RATE 200

typedef enum {
    GAME_LAUNCHED,          /* When a game is first started, roles need to be chosen */
    PITCHER_CHOOSE,         /* Pitcher is choosing what lane to throw the ball down */
    PITCHER_TIMING,         /* Pitcher is timing the power of his shot */
    PITCHER_BALL_THROWN,    /* Pitcher has thrown the ball and is waiting for batter's response */
    BATTER_IDLE,            /* Batter is waiting for the ball to be thrown */
    BATTER_BALL_THROWN      /* Ball comes towards batter for them to swing at */
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


static game_state_t game_state = PITCHER_CHOOSE; // Should be set to GAME_LAUNCHED initally

static pitcher_t pitcher = {.col = 2, .row = 3}; 
static power_bar_t power_bar = {.power = 0, .increment_value = 1};
static batter_t batter = {.col = 2, .row = 6};

// IR values
static uint8_t chosen_pitch_col;
static uint8_t chosen_pitch_power;

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
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
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
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
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
    case BATTER_IDLE:
        if (batter.col < LEDMAT_COLS_NUM - 2)
            batter.col++;
        break;    
    case BATTER_BALL_THROWN:
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
    case BATTER_IDLE:
        if (batter.col > 0)
            batter.col--;
        break;    
    case BATTER_BALL_THROWN:
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
        break;
    case PITCHER_CHOOSE:
        chosen_pitch_col = pitcher.col;
        power_bar.power = 0;
        power_bar.increment_value = 1;
        game_state = PITCHER_TIMING;
        break;    
    case PITCHER_TIMING:        
        chosen_pitch_power = power_bar.power;
        // game_state = PITCHER_BALL_THROWN;
        ball_packet = (chosen_pitch_col << 3) | chosen_pitch_power;
        ir_uart_putc(ball_packet);

        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case BATTER_IDLE:
        break;    
    case BATTER_BALL_THROWN:
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
    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        break;    
    case PITCHER_TIMING:
        break;    
    case PITCHER_BALL_THROWN:
        break;    
    case BATTER_IDLE:

        if (ir_uart_read_ready_p()) {
            ball_packet = ir_uart_getc();
            if (ball_packet_valid_p) {
                chosen_pitch_col = ball_packet >> 3;
                chosen_pitch_power = ball_packet & 0b111;
            }            
        }

        break;    
    case BATTER_BALL_THROWN:
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

void draw_all()
{   
    // declare here because not allowed to inside of case blocks
    tinygl_point_t pitcher_point = {pitcher.col, pitcher.row};
    tinygl_point_t power_bar_left_point = {.x = 0};
    tinygl_point_t power_bar_right_point = {.x = LEDMAT_COLS_NUM - 1};
    tinygl_point_t batter_left_point = {batter.col, batter.row};
    tinygl_point_t batter_right_point = {batter.col + 1, batter.row};

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
    case BATTER_IDLE:
        // tinygl_draw_line(batter_left_point, batter_right_point, 1);
        display_pixel_set(chosen_pitch_col, chosen_pitch_power, 1);
        break;    
    case BATTER_BALL_THROWN:
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
    uint8_t ir_ticks;
    uint8_t draw_ticks;
    

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


        ir_ticks++;
        if (ir_ticks >= PACER_RATE/IR_CHECK_RATE) {
            check_ir();
            ir_ticks = 0;
        }

        draw_ticks++;
        if (draw_ticks >= PACER_RATE/DRAW_RATE) {
            draw_all();
            draw_ticks = 0;
        }
        
        tinygl_update ();
    }
}