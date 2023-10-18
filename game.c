#include "system.h"
#include "pio.h"
#include "timer.h"
#include "pacer.h"
#include "navswitch.h"
#include "led.h"
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
#include "emotions.h"

#define PACER_RATE 200
#define ACTION_RATE 100
#define DRAW_HARD_RATE 200
#define DRAW_LIGHT_RATE 50
#define POWER_BAR_UPDATE_RATE 20

game_state_t game_state = GAME_LAUNCHED;

tinygl_point_t pitcher;
batter_t batter;
tinygl_point_t fielder;
tinygl_point_t hit_ball;
ball_t pitched_ball;
runner_t runner;
power_bar_t power_bar;

uint8_t bases_covered;
uint8_t strikes;
bool winner;

uint8_t chosen_pitch_col;
uint8_t chosen_pitch_power;

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
            ball_packet = (chosen_pitch_col << 0x03) | chosen_pitch_power;
            ir_uart_putc(ball_packet);
            game_state = PITCHER_BALL_THROWN;
            break;
        default:
            break;
    }
}

/**
 * Checks whether there was a navswitch push event at NAVSWITCH_CHECK_RATE Hz
 * and calls the appropriate function
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

void check_ir(void)
{
    uint8_t ball_packet;
    uint8_t hit_packet;
    uint8_t winner_packet;

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
                if (ir_uart_getc() == END_GAME) {
                    if (ir_uart_read_ready_p()) {
                        if (bases_covered > ir_uart_getc()) {
                            winner = 1; // this player is the winner
                            led_set(LED1, 1);
                            ir_uart_putc(YOU_LOSE);
                        } else {
                            winner = 0;
                            ir_uart_putc(YOU_WIN);
                        }
                        game_state = GAME_OVER;
                    }
                }            
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
                } else if (hit_packet == STRIKED_OUT) {
                    if (strikes == 0) {
                        game_state = BATTER_IDLE;
                    } else {
                        game_state = GAME_OVER;       

                        ir_uart_putc(END_GAME);
                        ir_uart_putc(bases_covered);
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
                    pitched_ball.pos.x = LEDMAT_COLS_NUM - 1 - chosen_pitch_col;
                    pitched_ball.pos.y = 0;
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
        case GAME_OVER:
            if (ir_uart_read_ready_p()) {
                winner_packet = ir_uart_getc();
                if (winner_packet == YOU_LOSE) {
                    winner = 0;
                } else if (winner_packet == YOU_WIN) {
                    winner = 1;
                    led_set(LED1, 1);

                }         
            }
        default:
            break;
    }
}

void draw_hard(void)
{   
    tinygl_point_t power_bar_left_point = {.x = 0, .y = LEDMAT_ROWS_NUM - 1};
    tinygl_point_t power_bar_right_point = {.x = LEDMAT_COLS_NUM - 1, .y = LEDMAT_ROWS_NUM - 1};
    tinygl_point_t batter_right_point = {.x = batter.pos.x + batter.extra_width, .y = batter.pos.y};
    tinygl_point_t face_pixel;

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
            if (winner) {
                for (uint8_t i = 0; i < LEDMAT_COLS_NUM; i++) {
                    for (uint8_t j = 0; j < LEDMAT_ROWS_NUM; j++) {
                        face_pixel.x = i;
                        face_pixel.y = j;
                        if ((BITMAP_HAPPY[i] >> j) & 1) {
                            tinygl_draw_point(face_pixel, 1);
                        } else {
                            tinygl_draw_point(face_pixel, 0);
                        }
                    }
                }
            } else {
                for (uint8_t i = 0; i < LEDMAT_COLS_NUM; i++) {
                    for (uint8_t j = 0; j < LEDMAT_ROWS_NUM; j++) {
                        face_pixel.x = i;
                        face_pixel.y = j;
                        if ((BITMAP_SAD[i] >> j) & 1) {
                            tinygl_draw_point(face_pixel, 1);
                        } else {
                            tinygl_draw_point(face_pixel, 0);
                        }
                    }
                }
            }
            
            break;
        default:
            break;
    }
}

void draw_light(void)
{   
    switch (game_state) {
        case PITCHER_FIELDING:        
            tinygl_draw_point(hit_ball, 1);
            break;
        case BATTER_RUNNING:
            for (uint8_t base = 0; base < NUM_BASES; base++)
                tinygl_draw_point(BASES[base], 1);
            break;
        default:
            break;
    }
}

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

int main (void)
{   
    // Initialisation
    system_init ();
    pacer_init(PACER_RATE);
    tinygl_init (PACER_RATE);
    tinygl_font_set(&font3x5_1);
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set(TINYGL_TEXT_DIR_ROTATE);
    tinygl_text(" PUSH TO BAT");

    navswitch_init();
    led_init();
    led_set(LED1, 0);
    ir_uart_init();
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

    // Main game loop
    while (1) {
        pacer_wait();

        if (game_state != GAME_LAUNCHED) tinygl_clear();

        action_ticks++;
        if (action_ticks >= PACER_RATE/ACTION_RATE) {
            check_navswitch();
            check_collisions();
            check_ir();
            action_ticks = 0;
        }

        // state-specific tasks
        if (game_state == PITCHER_TIMING) {
            power_bar_update_ticks++;
            if (power_bar_update_ticks >= PACER_RATE/POWER_BAR_UPDATE_RATE) {
                pitcher_power_bar_update(&power_bar);
                power_bar_update_ticks = 0;
            }
        }
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