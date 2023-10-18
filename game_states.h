#ifndef GAME_STATES_H
#define GAME_STATES_H

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

#endif