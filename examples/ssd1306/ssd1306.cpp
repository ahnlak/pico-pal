/*
 * ssd1306.cpp - example for for pico-pal
 */

/* System / Pico headers. */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* The required pico-pal libraries. */
#include "pal-ssd1306.h"

/* 
 * Ports and pins; SDA/SDL is usually GPIO8/9, but on the Pico Explorer these
 * are moved to GPIO20/21
 */

#define I2C_PORT    i2c0
#define I2C_SDA     20
#define I2C_SCL     21

#define OLED_WIDTH  128
#define OLED_HEIGHT 32

/*
 * A simple main() function to draw some graphics primitives on an eternal
 * loop.
 */

int main()
{
    /* Initialise the stdio stuff, handy for debugs. */
    stdio_init_all();

    /* Initialise the i2c stuff; baudrate is a semi-standard 400kHz */
    i2c_init( I2C_PORT, 400*1000 );
    
    gpio_set_function( I2C_SDA, GPIO_FUNC_I2C );
    gpio_set_function( I2C_SCL, GPIO_FUNC_I2C );
    gpio_pull_up( I2C_SDA );
    gpio_pull_up( I2C_SCL );

    /* Create a SSD1306 display object on that port. */
    pal::SSD1306 *l_display = new pal::SSD1306( OLED_WIDTH, OLED_HEIGHT, I2C_PORT, 0x3C );

for (;;) {
    l_display->clear();
    l_display->draw_box( 0, 0, OLED_WIDTH/2, OLED_HEIGHT-1, true );
    l_display->draw_text( 0,  0, "ABCDEFGHIJKLMNOPQRSTU" );
    l_display->draw_text( 0,  8, "VWXYZ:0123456789; <=>", false );
    l_display->draw_text( 0, 16, "abcdefghijklmnopqrstu" );
    l_display->draw_text( 0, 24, "vwxyz~({@}) !#$%^&*\"'" );
    l_display->render();
    sleep_ms( 1000 );
}

    /* And enter a permanent loop drawing on it. */
    for (;;) {

        uint8_t l_mid = ( OLED_WIDTH / 2 ) - 1;

        /* Open the shape. */
        l_display->set_contrast( 255 );
        l_display->set_invert( false );
        for( int x = 0; x < l_mid; x++ )
        {
            /* Clear the display. */
            l_display->clear();

            /* Draw vertical bars. */
            l_display->draw_line( l_mid - x, 0, l_mid - x, OLED_HEIGHT-1 );
            l_display->draw_line( l_mid + x, 0, l_mid + x, OLED_HEIGHT-1 );

            /* And some cross-parts. */
            l_display->draw_line( l_mid - x, 0, l_mid + x, OLED_HEIGHT-1 );
            l_display->draw_line( l_mid - x, OLED_HEIGHT-1, l_mid + x, 0 );

            /* And some text. */
            l_display->draw_text( 0, 0, "SSD1306 Driver" );

            /* Render the display. */
            l_display->render();

            /* And pause a little. */
            sleep_ms( 10 );
        }

        /* And close the shape. */
        l_display->set_contrast( 5 );
        l_display->set_invert( true );
        for( int x = l_mid; x > 0; x-- )
        {
            /* Clear the display. */
            l_display->clear();

            /* Draw vertical bars. */
            l_display->draw_line( l_mid - x, 0, l_mid - x, OLED_HEIGHT-1 );
            l_display->draw_line( l_mid + x, 0, l_mid + x, OLED_HEIGHT-1 );

            /* And some cross-parts. */
            l_display->draw_line( l_mid - x, 0, l_mid + x, OLED_HEIGHT-1 );
            l_display->draw_line( l_mid - x, OLED_HEIGHT-1, l_mid + x, 0 );

            /* Render the display. */
            l_display->render();

            /* And pause a little. */
            sleep_ms( 10 );
        }
    }

    /* We should never get here... */
    return 0;
}

/* End of examples/ssd1306/ssd1306.cpp */
