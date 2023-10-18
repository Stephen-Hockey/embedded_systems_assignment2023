/** @file graphics.h
 *  @authors Stephen Hockey, Xinwei Wang
 *  @date 19 Oct 2023
 *  @brief Helps draw more cumbersome graphics
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "system.h"
#include "tinygl.h"

extern const char LAUNCH_MSG[];

/**
 * A bitmap of a smiley face
*/
extern const uint8_t BITMAP_HAPPY[];

/**
 * A bitmap of a sad face
*/
extern const uint8_t BITMAP_SAD[];

/**
 * Draws the given bitmap onto the display.
*/
void draw_bitmap(const uint8_t bitmap[]);

#endif
