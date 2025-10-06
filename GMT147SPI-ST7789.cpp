#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lib/oled/st7789.h"  // OLED display library
#include "lib/oled/gfx.h"     // Graphics library for OLED
#include "lib/oled/gfxfont.h" // Font definitions for graphics library
#include "lib/hardware.h"     // Hardware configuration for OLED

/** @brief Display dimensions for GMT147SPI display of 172x320 ST7789 LCD */
const int lcd_width = 172;  ///< Display width in pixels
const int lcd_height = 320; ///< Display height in pixels

int main()
{
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to initialize
    // configure the ST7789 display
    // Configure SPI peripheral instance in line with hardware.h definitions
    spi_inst_t *st7789_spi = spi1;
    printf("SPI peripheral set to spi1\n");

    // Configure GPIO pins for ST7789 display
    // Parameters: DC, CS, RST, SCK, TX (MOSI)
    LCD_setPins(OLED_DC_PIN, OLED_CS_PIN, OLED_RESET_PIN, OLED_SCK_PIN, OLED_SDA_PIN);
    printf("ST7789 display pins configured\n");

    // Assign SPI peripheral to LCD driver
    LCD_setSPIperiph(st7789_spi);
    printf("SPI peripheral configured for LCD\n");
    printf("Initializing display...\n");
    // Initialize display with automatic offset correction for 170x320 displays
    LCD_initDisplay(lcd_width, lcd_height);
    printf("Display initialized with %dx%d resolution\n", lcd_width, lcd_height);

    // Set display orientation - 180° rotation for correct viewing
    LCD_setRotation(2);
    printf("Display rotation set to 180 degrees\n");

    printf("Creating framebuffer...\n");
    // Allocate framebuffer memory for graphics operations
    GFX_createFramebuf();
    printf("Framebuffer created successfully\n");

    // ✅ Clear the screen first before drawing
    printf("Clearing display...\n");
    GFX_fillScreen(ST77XX_BLACK); // Fill with black
    GFX_flush();                  // Send to display immediately
    sleep_ms(100);                // Let it settle
    printf("Drawing text...\n");
    GFX_setTextSize(2);
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(10, 10);
    GFX_printf("Hello ST7789!");
    GFX_flush();
    printf("Text drawn!\n");
    // ✅ REQUIRED: Infinite loop to prevent main() from returning
    printf("Entering main loop...\n");
    while (1)
    {
        tight_loop_contents(); // Hint to SDK that this is intentional
        sleep_ms(100);         // Small delay to prevent busy-waiting
    }
}
