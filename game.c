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

#define PACER_RATE 200
#define NAVSWITCH_CHECK_RATE 200
#define DRAW_RATE 200
#define POWER_BAR_UPDATE_RATE 10

typedef enum {
    GAME_LAUNCHED,          /* When a game is first started, roles need to be chosen */
    PITCHER_CHOOSE,         /* Pitcher is choosing what lane to throw the ball down */
    PITCHER_TIMING,         /* Pitcher is timing the power of his shot */
    PITCHER_BALL_THROWN,    /* Pitcher has thrown the ball and is waiting for batter's response */
    BATTER_IDLE,            /* Batter is waiting for the ball to be thrown */
    BATTER_BALL_THROWN      /* Ball comes towards batter for them to swing at */
    // TODO what happens after ball is hit/missed
} game_state_t;

typedef struct {
    uint8_t col;
    const uint8_t row;
} pitcher_t;

typedef struct {
    uint8_t power;
    int8_t increment_value;
} power_bar_t;


static game_state_t game_state = PITCHER_CHOOSE; // Should be set to GAME_LAUNCHED initally

static pitcher_t pitcher = {2, 3};

static power_bar_t power_bar = {1, 1};

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
        break;    
    case BATTER_BALL_THROWN:
        break;        
    default:
        break;
    }
}

void navswitch_push_pushed()
{
    switch (game_state) {
    case GAME_LAUNCHED:
        break;
    case PITCHER_CHOOSE:
        power_bar.power = 0;
        power_bar.increment_value = 1;
        game_state = PITCHER_TIMING;
        break;    
    case PITCHER_TIMING:        
        game_state = PITCHER_BALL_THROWN;
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
    tinygl_point_t pitcher_point = {pitcher.col, pitcher.row};
    tinygl_point_t power_bar_left_point = {.x = 0};
    tinygl_point_t power_bar_right_point = {.x = LEDMAT_COLS_NUM - 1};
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
    // led_init();

    // Declare tick counters
    uint8_t navswitch_check_ticks;
    uint8_t power_bar_update_ticks;
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
        

        draw_ticks++;
        if (draw_ticks >= PACER_RATE/DRAW_RATE) {
            draw_all();
            draw_ticks = 0;
        }
        
        tinygl_update ();
    }
}