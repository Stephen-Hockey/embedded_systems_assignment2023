/** @file graphics.c
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief A file that helps abstract some of the more complex output to the LED matrix
*/

#include "graphics.h"

// The text that displays upon launch
const char LAUNCH_MSG[] = " PUSH TO BAT";

/**
 * A bitmap of a smiley face
*/
const uint8_t BITMAP_HAPPY[] =
{
    0b0011000,
    0b0100010,
    0b0100000,
    0b0100010,
    0b0011000
};

/**
 * A bitmap of a sad face
*/
const uint8_t BITMAP_SAD[] =
{
    0b0110000,
    0b0001010,
    0b0001000,
    0b0001010,
    0b0110000
};

/**
 * Draws the given bitmap onto the display.
*/
void draw_bitmap(const uint8_t bitmap[])
{
    tinygl_point_t bitmap_point;
    for (uint8_t i = 0; i < LEDMAT_COLS_NUM; i++) {
        for (uint8_t j = 0; j < LEDMAT_ROWS_NUM; j++) {
            bitmap_point.x = i;
            bitmap_point.y = j;
            if ((bitmap[i] >> j) & 1) {
                tinygl_draw_point(bitmap_point, 1);
            } else {
                tinygl_draw_point(bitmap_point, 0);
            }
        }
    }
}