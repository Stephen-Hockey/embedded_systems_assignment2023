/** @file game.c
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief The main file of a baseball-themed game
*/

#include "system.h"
#include "pio.h"
#include "timer.h"
#include "pacer.h"
#include "navswitch.h"
#include "ledmat.h"
#include "display.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "ir_uart.h"
#include "baseball_objects.h"
#include "game_states.h"
#include "pitcher.h"
#include "batter.h"
#include "ir_handler.h"
#include "graphics.h"

// Rates for the pacer in Hz
#define PACER_RATE 200
#define ACTION_RATE 100
#define DRAW_HARD_RATE 200
#define DRAW_LIGHT_RATE 50
#define POWER_BAR_UPDATE_RATE 25

// The state that the FunKit is currently in. This allows for many different roles and types of gameplay
game_state_t game_state = GAME_LAUNCHED;

// The moving parts that need to displayed on the LED matrix
tinygl_point_t pitcher;
batter_t batter;
tinygl_point_t fielder;
tinygl_point_t hit_ball;
ball_t pitched_ball;
runner_t runner;
power_bar_t power_bar;

// Game scores. Winner is decided based on bases_covered
uint8_t bases_covered;
uint8_t strikes;
bool is_winner;

// The chosen pitch column and power of a given pitch
uint8_t chosen_pitch_col;
uint8_t chosen_pitch_power;

/**
 * Controls what happens when the north navswitch button is pressed
*/
void navswitch_north_pushed(void)
{
    switch (game_state) {
        case PITCHER_FIELDING:
            fielder_move_up(&fielder);
            break;
        case BATTER_BALL_THROWN:
            if (ball_hit_p(pitched_ball, batter)) {
                runner_init(&runner);
                game_state = BATTER_RUNNING;
                ir_uart_putc(BALL_HIT);
            }
            break;        
        case BATTER_RUNNING:
            runner_move_up(&runner);
            break;
        default:
            break;
    }
}

/**
 * Controls what happens when the south navswitch button is pressed
*/
void navswitch_south_pushed(void)
{
    switch (game_state) {
        case PITCHER_FIELDING:
            fielder_move_down(&fielder);      
            break;
        case BATTER_RUNNING:
            runner_move_down(&runner);
            break;
        default:
            break;
    }
}

/**
 * Controls what happens when the east navswitch button is pressed
*/
void navswitch_east_pushed(void)
{
    switch (game_state) {
        case PITCHER_CHOOSE:
            pitcher_move_right(&pitcher);
            break;    
        case PITCHER_FIELDING:
            fielder_move_right(&fielder);
            break;
        case BATTER_IDLE:
            batter_move_right(&batter);
            break;    
        case BATTER_BALL_THROWN:
            batter_move_right(&batter);
            break;   
        case BATTER_RUNNING:
            runner_move_right(&runner);
            break;
        default:
            break;
    }
}

/**
 * Controls what happens when the west navswitch button is pressed
*/
void navswitch_west_pushed(void)
{   
    switch (game_state) {
        case PITCHER_CHOOSE:
            pitcher_move_left(&pitcher);
            break;  
        case PITCHER_FIELDING:
            fielder_move_left(&fielder);
            break;
        case BATTER_IDLE:
            batter_move_left(&batter);
            break;    
        case BATTER_BALL_THROWN:
            batter_move_left(&batter);
            break;        
        case BATTER_RUNNING:
            runner_move_left(&runner);
            break;
        default:
            break;
    }
}

/**
 * Controls what happens when the navswitch push button is pressed
*/
void navswitch_push_pushed(void)
{
    uint8_t ball_packet; 
    switch (game_state) {
        case GAME_LAUNCHED:
            ir_uart_putc(START_GAME); 
            game_state = BATTER_IDLE;        
            break;
        case PITCHER_CHOOSE:
            chosen_pitch_col = pitcher.x;
            pitcher_power_bar_init(&power_bar);
            game_state = PITCHER_TIMING;
            break;    
        case PITCHER_TIMING:
            chosen_pitch_power = power_bar.power;

            // Concatenates the chosen_pitch_col with the chosen_pitch_power in binary.
            ball_packet = (chosen_pitch_col << 0x03) | chosen_pitch_power;

            ir_uart_putc(ball_packet);
            game_state = PITCHER_BALL_THROWN;
            break;
        case GAME_OVER:
            // Restart Game
            strikes = 0;
            bases_covered = 0;
            runner.base_num = 0;
            tinygl_text(LAUNCH_MSG);   
            game_state = GAME_LAUNCHED;
        default:
            break;
    }
}

/**
 * Checks whether there was a navswitch push event and redirects to the appropriate function
*/
void check_navswitch(void) 
{
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

/**
 * Checks whether an object has touched another.
 * This is important for the fielder touching the hit ball, and the runner touching the bases.
*/
void check_collisions(void)
{
    switch (game_state) {
        case PITCHER_FIELDING:
            if (point_equals_p(fielder, hit_ball)) {
                ir_uart_putc(BALL_FIELDED);
                pitcher_init(&pitcher);
                game_state = PITCHER_CHOOSE;
            }
            break;
        case BATTER_RUNNING:
            // Check for when the runner touches the bases, but only change runner.base_num if the bases are touched in order
            for (uint8_t base = 0; base < NUM_BASES; base++) {
                if (point_equals_p(runner.pos, BASES[(base + 1) % NUM_BASES]) && runner.base_num == base) {
                    runner.base_num = (runner.base_num + 1) % NUM_BASES;
                    bases_covered++;
                }
            }
            break;
        default:
            break;
    }
}

/**
 * Checks for IR message reception and takes the appropriate action
*/
void check_ir(void)
{
    uint8_t ball_packet;
    uint8_t hit_packet;

    switch (game_state) {
        case GAME_LAUNCHED:
            if (ir_uart_read_ready_p()) {
                if (ir_uart_getc() == START_GAME) {
                    game_state = PITCHER_CHOOSE;
                }            
            }
            break;
        case PITCHER_CHOOSE:
            if (ir_uart_read_ready_p()) {
                if (bases_covered > ir_uart_getc()) {
                    is_winner = 1;
                    ir_uart_putc(YOU_LOSE);
                } else {
                    is_winner = 0;
                    ir_uart_putc(YOU_WIN);
                }
                game_state = GAME_OVER;
            }
            break;   
        case PITCHER_BALL_THROWN:
            if (ir_uart_read_ready_p()) {
                hit_packet = ir_uart_getc();
                if (hit_packet == BALL_HIT) {
                    fielder_init(&fielder, &hit_ball);
                    game_state = PITCHER_FIELDING;
                } else if (hit_packet == BALL_MISSED) {
                    pitcher_init(&pitcher);
                    game_state = PITCHER_CHOOSE;
                } else if (hit_packet == STRIKED_OUT && strikes == 0) {
                    game_state = BATTER_IDLE;
                } else if (hit_packet == STRIKED_OUT && strikes > 0) {
                    ir_uart_putc(bases_covered);
                } else if (hit_packet == YOU_LOSE) {
                    is_winner = 0;
                    game_state = GAME_OVER;
                } else if (hit_packet == YOU_WIN) {
                    is_winner = 1;
                    game_state = GAME_OVER;
                }
            }
            break;
        case BATTER_IDLE:
            if (ir_uart_read_ready_p()) {
                ball_packet = ir_uart_getc();
                if (ball_packet_valid_p(ball_packet)) {

                    // Unpackage 'packet' into the two values it represents
                    chosen_pitch_col = ball_packet >> 0x03;
                    chosen_pitch_power = ball_packet & 0x07;

                    // Makes sure the ball is displayed on the opposite side, as the 2 funkits are opposite orientation
                    pitched_ball.pos.x = LEDMAT_COLS_NUM - 1 - chosen_pitch_col;
                    pitched_ball.pos.y = 0;

                    // using a switch here to manually set the numbers allows for skill to be better rewarded
                    switch (chosen_pitch_power)
                    {
                    case 0:
                        pitched_ball.move_rate = 2;
                        break; 
                    case 1:
                    case 2:
                    case 3:
                        pitched_ball.move_rate = 5;
                        break;    
                    case 4:
                        pitched_ball.move_rate = 10;
                    case 5:
                        pitched_ball.move_rate = 20;
                        break;    
                    case 6:
                        pitched_ball.move_rate = 40;
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
                if (ir_uart_getc() == BALL_FIELDED) {
                    batter_init(&batter);
                    game_state = BATTER_IDLE;
                }           
            }
            break;
        default:
            break;
    }
}

/**
 * Displays what needs to be displayed as solid LED, as opposed to flickering. This is most of the display.
*/
void draw_hard(void)
{   
    // Not allowed in case blocks
    tinygl_point_t power_bar_left_point = {.x = 0, .y = LEDMAT_ROWS_NUM - 1};
    tinygl_point_t power_bar_right_point = {.x = LEDMAT_COLS_NUM - 1, .y = LEDMAT_ROWS_NUM - 1};
    tinygl_point_t batter_right_point = {.x = batter.pos.x + batter.extra_width, .y = batter.pos.y};

    switch (game_state) {
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
            tinygl_draw_point(pitched_ball.pos, 1);
            tinygl_draw_line(batter.pos, batter_right_point, 1);
            break;        
        case BATTER_RUNNING:
            tinygl_draw_point(runner.pos, 1);
            break;
        case GAME_OVER:
            if (is_winner) {
                draw_bitmap(BITMAP_HAPPY);
            } else {
                draw_bitmap(BITMAP_SAD);
            }            
            break;
        default:
            break;
    }
}

/**
 * Displays things at a lower frequency, achieving an effect that distinguishes some objects from others.
 * It is like having two colours. This is used to draw the bases and the hit ball.
*/
void draw_light(void)
{   
    switch (game_state) {
        case PITCHER_FIELDING:
            tinygl_draw_point(hit_ball, 1);
            break;
        case BATTER_RUNNING:
            // Display all four bases
            for (uint8_t base = 0; base < NUM_BASES; base++)
                tinygl_draw_point(BASES[base], 1);
            break;
        default:
            break;
    }
}

/**
 * Updates the location of the ball as the batter sees it being pitched at him.
 * Triggers a strike once off the screen.
*/
void update_pitched_ball(void)
{
    if (pitched_ball.pos.y < LEDMAT_ROWS_NUM) { // intentionally goes to 7
        pitched_ball.pos.y++;
    } else {
        strikes++;
        if (strikes == MAX_STRIKES) {
            game_state = PITCHER_CHOOSE;
            ir_uart_putc(STRIKED_OUT);
            return;
        }
        ir_uart_putc(BALL_MISSED);
        batter_init(&batter);
        game_state = BATTER_IDLE;
    }        
}

/**
 * The main program function
*/
int main (void)
{   
    // Initialisation of modules
    system_init ();
    pacer_init(PACER_RATE);
    tinygl_init (PACER_RATE);
    tinygl_font_set(&font3x5_1);
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set(TINYGL_TEXT_DIR_ROTATE);
    tinygl_text(LAUNCH_MSG);    
    navswitch_init();
    ir_uart_init();
    
    //Initialisation of baseball objects and scores
    pitcher_init(&pitcher);
    batter_init(&batter);
    runner.base_num = 0;
    runner_init(&runner);
    bases_covered = 0;
    strikes = 0;

    // Declare tick counters
    uint8_t action_ticks = 0;
    uint8_t power_bar_update_ticks = 0;
    uint8_t pitched_ball_ticks = 0;
    uint8_t draw_hard_ticks = 0;
    uint8_t draw_light_ticks = 0;

    // Main game loop that utilizes a pacer architecture
    while (1) {
        pacer_wait();

        // Clearing the display when text is being displayed doesn't work very well.
        if (game_state != GAME_LAUNCHED) tinygl_clear();

        action_ticks++;
        if (action_ticks >= PACER_RATE/ACTION_RATE) {
            check_navswitch();
            check_collisions();
            check_ir();
            action_ticks = 0;
        }

        // state-specific task
        if (game_state == PITCHER_TIMING) {
            power_bar_update_ticks++;
            if (power_bar_update_ticks >= PACER_RATE/POWER_BAR_UPDATE_RATE) {
                pitcher_power_bar_update(&power_bar);
                power_bar_update_ticks = 0;
            }
        }

        // state-specific task
        if (game_state == BATTER_BALL_THROWN) {
            pitched_ball_ticks++;
            if (pitched_ball_ticks >= PACER_RATE/pitched_ball.move_rate) {
                update_pitched_ball();
                pitched_ball_ticks = 0;
            }
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