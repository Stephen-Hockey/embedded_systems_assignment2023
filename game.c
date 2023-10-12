#include "system.h"
#include "pio.h"
#include "pacer.h"

static uint8_t previous_column;

/** Define PIO pins driving LED matrix rows.  */
static const pio_t rows[] =
{
    LEDMAT_ROW1_PIO, LEDMAT_ROW2_PIO, LEDMAT_ROW3_PIO, 
    LEDMAT_ROW4_PIO, LEDMAT_ROW5_PIO, LEDMAT_ROW6_PIO,
    LEDMAT_ROW7_PIO
};


/** Define PIO pins driving LED matrix columns.  */
static const pio_t cols[] =
{
    LEDMAT_COL1_PIO, LEDMAT_COL2_PIO, LEDMAT_COL3_PIO,
    LEDMAT_COL4_PIO, LEDMAT_COL5_PIO
};


static const uint8_t bitmap[] =
{
    0b0011100,
    0b0110000,
    0b0111100,
    0b0011100,
    0b0010100
};

static void display_column (uint8_t row_pattern, uint8_t current_column)
{

    /* TODO */
    pio_output_high(cols[previous_column]);
    for (uint8_t i = 0; i < LEDMAT_ROWS_NUM; i++) {
        if ((row_pattern >> i) & 1) {
            pio_output_low(rows[i]);
        } else {
            pio_output_high(rows[i]);
        }
    }
    pio_output_low(cols[current_column]);
    previous_column = current_column;

}


int main (void)
{
    uint8_t current_column = 0;
  
    system_init ();
    pacer_init (500);
    
    /* TODO: Initialise LED matrix pins.  */
    for (uint8_t i = 0; i < LEDMAT_COLS_NUM; i++) {
        pio_config_set(cols[i], PIO_OUTPUT_HIGH);
    }
    for (uint8_t i = 0; i < LEDMAT_ROWS_NUM; i++) {
        pio_config_set(rows[i], PIO_OUTPUT_HIGH);
    }

    while (1)
    {
        pacer_wait();
        
        display_column(bitmap[current_column], current_column);
    
        current_column++;
    
        if (current_column > (LEDMAT_COLS_NUM - 1))
        {
            current_column = 0;
        }           
    }
}
