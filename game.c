#include <stdlib.h>
#include "system.h"
#include "pio.h"
#include "timer.h"
#include "pacer.h"
#include "navswitch.h"
#include "led.h"
#include "ledmat.h"
#include "display.h"
#include "tinygl.h"
#include "font.h"
#include "ir_uart.h"
#include "baseball_objects.h"
#include "game_states.h"
#include "pitcher.h"
#include "batter.h"

#define PACER_RATE 200
#define NAVSWITCH_CHECK_RATE 100
#define COLLISION_CHECK_RATE 100
#define IR_CHECK_RATE 100
#define DRAW_HARD_RATE 200
#define DRAW_LIGHT_RATE 50
#define POWER_BAR_UPDATE_RATE 20

game_state_t game_state = GAME_LAUNCHED;

tinygl_point_t pitcher;
batter_t batter;
tinygl_point_t fielder;
tinygl_point_t hit_ball;
ball_t pitched_ball;
runner_t runner = {.base_num = 0};
power_bar_t power_bar;

uint16_t bases_covered;
uint8_t strikes;

uint8_t chosen_pitch_col;
uint8_t chosen_pitch_power;

void navswitch_north_pushed()
{
    switch (game_state) {
        case PITCHER_FIELDING:
            fielder_move_up(&fielder);
            break;
        case BATTER_BALL_THROWN:
            if (ball_hit_p(pitched_ball, batter)) {
                runner_init(&runner);
                game_state = BATTER_RUNNING;
                ir_uart_putc('O');
            }
            break;        
        case BATTER_RUNNING:
            runner_move_up(&runner);
            break;
    }
}

void navswitch_south_pushed()
{
    switch (game_state) {
        case PITCHER_FIELDING:
            fielder_move_down(&fielder);      
            break;
        case BATTER_RUNNING:
            runner_move_down(&runner);
            break;
    }
}

void navswitch_east_pushed()
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
    }
}

void navswitch_west_pushed()
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
            ball_packet = (chosen_pitch_col << 0x03) | chosen_pitch_power;
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
            if (point_equals(fielder, hit_ball)) {
                ir_uart_putc('F');
                pitcher_init(&pitcher);
                game_state = PITCHER_CHOOSE;
            }
            break;
        case BATTER_RUNNING:
            for (uint8_t base = 0; base < NUM_BASES; base++) {
                if (point_equals(runner.pos, BASES[(base + 1) % NUM_BASES]) && runner.base_num == base) {
                    runner.base_num = (runner.base_num + 1) % NUM_BASES;
                    bases_covered++;
                }
            }
            break;
    }
}

void check_ir()
{
    uint8_t ball_packet;
    uint8_t hit_packet;

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
                    pitcher_init(&pitcher);
                    game_state = PITCHER_CHOOSE;
                } else if (hit_packet == 'O') {
                    fielder_init(&fielder, &hit_ball);
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
                if (ir_uart_getc() == 'F') {
                    batter_init(&batter);
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

void update_pitched_ball()
{
    if (pitched_ball.pos.y < LEDMAT_ROWS_NUM) { // intentionally goes to 7
        pitched_ball.pos.y++;
    } else {
        strikes++;
        if (strikes == MAX_STRIKES) {
            game_state = PITCHER_CHOOSE;
            ir_uart_putc('S');
            return;
        }
        ir_uart_putc('X');
        batter_init(&batter);
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
            tinygl_draw_point(pitched_ball.pos, 1);
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
            tinygl_draw_point(hit_ball, 1);
            break;
        case BATTER_RUNNING:
            for (uint8_t base = 0; base < NUM_BASES; base++)
                tinygl_draw_point(BASES[base], 1);
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
    led_init();
    led_set(LED1, 0);
    ir_uart_init();

    pitcher_init(&pitcher);
    batter_init(&batter);

    // Declare tick counters
    uint8_t navswitch_check_ticks;
    uint8_t collision_check_ticks;
    uint8_t power_bar_update_ticks;
    uint8_t pitched_ball_ticks;
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

        if (game_state == PITCHER_TIMING) {
            power_bar_update_ticks++;
            if (power_bar_update_ticks >= PACER_RATE/POWER_BAR_UPDATE_RATE) {
                update_power_bar();
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