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
#define NAVSWITCH_CHECK_RATE 50


typedef enum
{
    GAME_LAUNCHED,          /* When a game is first started, roles need to be chosen */
    PITCHER_CHOOSE,         /* Pitcher is choosing what lane to throw the ball down */
    PITCHER_TIMING,         /* Pitcher is timing the power of his shot */
    PITCHER_BALL_THROWN,    /* Pitcher has thrown the ball and is waiting for batter's response */
    BATTER_IDLE,            /* Batter is waiting for the ball to be thrown */
    BATTER_BALL_THROWN      /* Ball comes towards batter for them to swing at */
} game_state_t;

static game_state_t game_state = GAME_LAUNCHED;

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
        navswitch_pushed();
}

void navswitch_north_pushed()
{
    
}

void navswitch_south_pushed()
{
    
}

void navswitch_east_pushed()
{
    
}

void navswitch_west_pushed()
{

}

void navswitch_pushed()
{
    
}

int main (void)
{   
    // Initialisation
    system_init ();
    pacer_init(PACER_RATE);
    tinygl_init(PACER_RATE);

    // Declare tick counters
    uint8_t navswitch_check_ticks;

    // Main game loop
    while (1) {
        pacer_wait();
        tinygl_clear();

        navswitch_check_ticks++;
        if (navswitch_check_ticks >= PACER_RATE/NAVSWITCH_CHECK_RATE) {
            check_navswitch();
            navswitch_check_ticks = 0;
        }

        tinygl_update ();
    }
}