/**
 * @file GMT147SPI-ST7789.cpp
 * @brief Test program for ST7789P3 display driver on Raspberry Pi Pico
 * @author Alejandro Moglia (@bartola-valves)
 * @date 6th Oct 2025
 *
 * WORKING BASELINE CONFIGURATION for ST7789P3 172x320 display
 * Hardware: Raspberry Pi Pico + ST7789P3 1.47" TFT LCD (172x320)
 * SPI Speed: 4 MHz (breadboard tested)
 * SPI Mode: Mode 0 (CPOL_0, CPHA_0) - CRITICAL for ST7789P3
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lib/oled/st7789.h"  // OLED display library
#include "lib/oled/gfx.h"     // Graphics library for OLED
#include "lib/oled/gfxfont.h" // Font definitions for graphics library
#include "lib/hardware.h"     // Hardware configuration for OLED

/** @brief Display dimensions for ST7789P3 1.47" display (172x320 pixels) */
const int lcd_width = 172;  ///< Display width in pixels (ST7789P3 variant)
const int lcd_height = 320; ///< Display height in pixels

/**
 * @brief Test function to determine character capacity of the display
 * @param textSize Font size multiplier (1 = 6x8 pixels per char, 2 = 12x16, etc.)
 *
 * This function calculates and displays how many characters fit on the screen
 * at a given text size. It fills the screen with numbered rows to visually
 * verify the calculation.
 *
 * Default font dimensions: 6 pixels wide × 8 pixels tall (size 1)
 * Actual char dimensions = (6 * textSize) × (8 * textSize)
 *
 * RECOMMENDATIONS for 1.47" ST7789P3 display:
 * - Size 1: Too small, hard to read (28 chars × 40 rows = 1120 chars)
 * - Size 2: RECOMMENDED - Good readability (14 chars × 20 rows = 280 chars) ✅
 * - Size 3: Large, limited space (9 chars × 13 rows = 117 chars)
 */
void testCharacterCapacity(uint8_t textSize)
{
    // Font characteristics (default built-in font)
    const int charWidth = 6;    // pixels per character (width)
    const int charHeight = 8;   // pixels per character (height)
    const int SAFE_MARGIN = 20; // pixels from edge (safe zone for rounded corners)

    // Calculate actual dimensions with text size multiplier
    int actualCharWidth = charWidth * textSize;
    int actualCharHeight = charHeight * textSize;

    // Calculate safe display area (accounting for 20px margins on all sides)
    int safeWidth = lcd_width - (SAFE_MARGIN * 2);
    int safeHeight = lcd_height - (SAFE_MARGIN * 2);

    // Calculate how many characters fit in safe area
    int charsPerRow = safeWidth / actualCharWidth;
    int numRows = safeHeight / actualCharHeight;

    // Display results on serial
    printf("\n=== Character Capacity Test (Safe Zone) ===\n");
    printf("Display: %dx%d pixels\n", lcd_width, lcd_height);
    printf("Safe zone: %dx%d pixels (20px margins)\n", safeWidth, safeHeight);
    printf("Text size: %d\n", textSize);
    printf("Character dimensions: %dx%d pixels\n", actualCharWidth, actualCharHeight);
    printf("Characters per row: %d\n", charsPerRow);
    printf("Number of rows: %d\n", numRows);
    printf("Total characters: %d\n", charsPerRow * numRows);
    printf("================================\n\n");

    // Clear screen
    GFX_fillScreen(ST77XX_BLACK);

    // Draw safe zone border for reference
    GFX_drawRect(SAFE_MARGIN, SAFE_MARGIN, safeWidth, safeHeight, ST77XX_YELLOW);

    // Set text properties
    GFX_setTextSize(textSize);
    GFX_setTextColor(ST77XX_WHITE);

    // Draw test pattern: fill safe zone with numbered rows
    printf("Drawing test pattern in safe zone...\n");
    for (int row = 0; row < numRows; row++)
    {
        // Position text within safe zone
        GFX_setCursor(SAFE_MARGIN, SAFE_MARGIN + (row * actualCharHeight));

        // Draw row number and fill with characters
        // Format: "R01:ABCDEFGHIJ..." where R01 is the row number
        char buffer[64];
        int written = snprintf(buffer, sizeof(buffer), "R%02d:", row);

        // Fill remaining space with alphabet characters
        int remaining = charsPerRow - written;
        for (int i = 0; i < remaining && (written + i) < (int)sizeof(buffer) - 1; i++)
        {
            buffer[written + i] = 'A' + (i % 26);
        }
        buffer[charsPerRow] = '\0'; // Null terminate

        GFX_printf("%s", buffer);

        // Show progress every 10 rows
        if ((row + 1) % 10 == 0)
        {
            printf("  Drawn %d rows...\n", row + 1);
        }
    }

    GFX_flush();
    printf("Test pattern complete!\n");
    printf("All rows should be fully visible within yellow border.\n");
}

/**
 * @brief Simple readability comparison test
 *
 * Shows the same sample text at different sizes for easy comparison.
 * Helps determine which text size is most readable for your use case.
 */
void testReadability()
{
    printf("\n=== Readability Comparison Test ===\n");

    GFX_fillScreen(ST77XX_BLACK);
    int yPos = 5;

    // Size 1 sample
    GFX_setTextSize(1);
    GFX_setTextColor(ST77XX_CYAN);
    GFX_setCursor(5, yPos);
    GFX_printf("Size1: Quick test");
    yPos += 15;
    printf("Size 1: 6x8 pixels - Very small\n");

    // Size 2 sample (RECOMMENDED)
    GFX_setTextSize(2);
    GFX_setTextColor(ST77XX_GREEN);
    GFX_setCursor(5, yPos);
    GFX_printf("Size2: Test");
    yPos += 25;
    printf("Size 2: 12x16 pixels - RECOMMENDED ✅\n");

    // Size 3 sample
    GFX_setTextSize(3);
    GFX_setTextColor(ST77XX_YELLOW);
    GFX_setCursor(5, yPos);
    GFX_printf("Size3:");
    yPos += 30;
    GFX_setCursor(5, yPos);
    GFX_printf("Test");
    yPos += 35;
    printf("Size 3: 18x24 pixels - Large\n");

    // Summary at bottom
    GFX_setTextSize(1);
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(5, 280);
    GFX_printf("Cyan=Sz1 Green=Sz2");
    GFX_setCursor(5, 290);
    GFX_printf("Yellow=Sz3");

    GFX_flush();
    printf("Readability test complete!\n");
    printf("Look at display to compare sizes.\n");
    printf("================================\n\n");
}

/**
 * @brief Test safe display area accounting for rounded corners
 *
 * The 1.47" ST7789P3 display has rounded corners that crop edge content.
 * This test determines the safe drawing area by testing borders.
 *
 * FINDINGS: R0 and R19 truncated, "Edge" at (0,0) overlaps "Safe Zone" at (10,10)
 * UPDATED: Testing 20px and 25px margins to find true safe zone
 */
void testSafeZone()
{
    printf("\n=== Safe Zone Test (Rounded Corners) ===\n");
    printf("Testing border visibility with multiple margins...\n");

    GFX_fillScreen(ST77XX_BLACK);

    // Draw border rectangles at different margins to find safe zone
    // Red border at edges (will be cropped)
    GFX_drawRect(0, 0, lcd_width, lcd_height, ST77XX_RED);
    GFX_drawRect(1, 1, lcd_width - 2, lcd_height - 2, ST77XX_RED);

    // Orange border at 10px (UNSAFE - confirmed overlapping)
    GFX_drawRect(10, 10, lcd_width - 20, lcd_height - 20, ST77XX_ORANGE);

    // Yellow border at 20px (TESTING - likely safe)
    GFX_drawRect(20, 20, lcd_width - 40, lcd_height - 40, ST77XX_YELLOW);
    GFX_drawRect(21, 21, lcd_width - 42, lcd_height - 42, ST77XX_YELLOW);

    // Green border at 25px (EXTRA SAFE)
    GFX_drawRect(25, 25, lcd_width - 50, lcd_height - 50, ST77XX_GREEN);

    // Test text at different positions
    GFX_setTextSize(2);

    // Position 1: (0,0) - Edge (will be cropped) - CYAN
    GFX_setTextColor(ST77XX_CYAN);
    GFX_setCursor(0, 0);
    GFX_printf("Edge");

    // Position 2: (10,10) - 10px margin (overlaps edge text) - MAGENTA
    GFX_setTextColor(ST77XX_MAGENTA);
    GFX_setCursor(10, 10);
    GFX_printf("10px");

    // Position 3: (20,40) - 20px margin (testing) - WHITE
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(20, 40);
    GFX_printf("20px Safe?");

    // Position 4: (25,70) - 25px margin (extra safe) - GREEN
    GFX_setTextColor(ST77XX_GREEN);
    GFX_setCursor(25, 70);
    GFX_printf("25px Safe!");

    // Bottom test
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(20, lcd_height - 40);
    GFX_printf("Bottom 20px");

    GFX_flush();

    printf("Border colors:\n");
    printf("  Red: Display edges (cropped by rounded corners)\n");
    printf("  Orange: 10px margin (UNSAFE - overlapping observed)\n");
    printf("  Yellow: 20px margin (TESTING)\n");
    printf("  Green: 25px margin (EXTRA SAFE)\n");
    printf("\nText positions:\n");
    printf("  Cyan 'Edge' at (0,0): Will be cropped\n");
    printf("  Magenta '10px' at (10,10): Overlaps with Edge text\n");
    printf("  White '20px Safe?' at (20,40): Testing\n");
    printf("  Green '25px Safe!' at (25,70): Extra safe\n");
    printf("================================\n\n");
}

/**
 * @brief Practical usage test - demonstrates typical text layout
 *
 * Shows a realistic multi-line text display using safe margins
 * UPDATED: Using 20px margin based on rounded corner findings
 */
void testPracticalLayout()
{
    printf("\n=== Practical Layout Test ===\n");
    printf("Simulating typical text display with 20px margins...\n");

    const int MARGIN = 20;      // Safe margin from edges (UPDATED from 10px)
    const int LINE_HEIGHT = 20; // Height for size 2 text (16 + 4 spacing)

    GFX_fillScreen(ST77XX_BLACK);
    GFX_setTextSize(2);

    int yPos = MARGIN;

    // Title
    GFX_setTextColor(ST77XX_YELLOW);
    GFX_setCursor(MARGIN, yPos);
    GFX_printf("ST7789 Test");
    yPos += LINE_HEIGHT + 5;

    // Info lines
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(MARGIN, yPos);
    GFX_printf("Display:");
    yPos += LINE_HEIGHT;

    GFX_setCursor(MARGIN, yPos);
    GFX_printf("172x320 px");
    yPos += LINE_HEIGHT;

    GFX_setCursor(MARGIN, yPos);
    GFX_printf("20px margin");
    yPos += LINE_HEIGHT + 10;

    // Status indicator
    GFX_setTextColor(ST77XX_GREEN);
    GFX_setCursor(MARGIN, yPos);
    GFX_printf("Safe Zone!");

    // Footer with row calculation
    GFX_setTextColor(ST77XX_CYAN);
    int footerY = lcd_height - MARGIN - 16; // 16 = height of size 2 text
    GFX_setCursor(MARGIN, footerY);

    // Calculate usable rows with safe margins
    int usableHeight = lcd_height - (2 * MARGIN);
    int usableRows = usableHeight / LINE_HEIGHT;
    GFX_printf("~%d rows", usableRows);

    GFX_flush();

    printf("Safe area dimensions:\n");
    printf("  Width: %d pixels (%d margin each side)\n", lcd_width - (2 * MARGIN), MARGIN);
    printf("  Height: %d pixels (%d margin top/bottom)\n", lcd_height - (2 * MARGIN), MARGIN);
    printf("  Usable rows (size 2, 20px spacing): %d\n", usableRows);
    printf("  Characters per row: ~14\n");
    printf("================================\n\n");
}

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

    // Set display orientation
    // Rotation values: 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
    LCD_setRotation(0); // ← Try 0 instead of 2
    printf("Display rotation set to 0 degrees\n");

    printf("Creating framebuffer...\n");
    // Allocate framebuffer memory for graphics operations
    GFX_createFramebuf();
    printf("Framebuffer created successfully\n");

    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║  ST7789P3 Safe Zone Tests - 172x320 Rounded Display  ║\n");
    printf("║  LOOPING TESTS - Watch for text/border overlap    ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║  UPDATED RECOMMENDATIONS for 1.47\" ST7789P3:      ║\n");
    printf("║  • Text Size: 2 (12x16 pixels) - MINIMUM USABLE   ║\n");
    printf("║  • Safe Margin: 20-25 pixels from edges           ║\n");
    printf("║    (10px UNSAFE - causes overlapping)             ║\n");
    printf("║  • Red box: Truncated (rounded corners)           ║\n");
    printf("║  • Orange box: Likely safe based on observations  ║\n");
    printf("║  • Usable Area: 132x280 pixels (20px margin)      ║\n");
    printf("║  • Practical Rows: ~14 (with 20px line spacing)   ║\n");
    printf("║  • Characters/Row: ~11 (with 20px margins)        ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    // ✅ REQUIRED: Infinite loop cycling through tests
    printf("Entering test loop (cycling every 8 seconds)...\n");
    uint8_t testNum = 0;
    while (1)
    {
        switch (testNum)
        {
        case 0:
            // Test 1: Safe zone visualization (shows rounded corner cropping)
            printf("\n▶ Test 1: Safe Zone & Border Test\n");
            printf("  Red = edges (cropped), Orange = 10px\n");
            printf("  Yellow = 20px, Green = 25px\n");
            printf("  OBSERVE: Text overlap with borders\n");
            testSafeZone();
            break;

        case 1:
            // Test 2: Practical layout example
            printf("\n▶ Test 2: Practical Multi-line Layout\n");
            testPracticalLayout();
            break;

        case 2:
            // Test 3: Full screen capacity with safe margins
            printf("\n▶ Test 3: Full Screen Capacity - Size 2\n");
            testCharacterCapacity(2);
            break;

        case 3:
            // Final demo: Updated safe position
            printf("\n▶ Test 4: Simple Demo at (20,20)\n");
            GFX_fillScreen(ST77XX_BLACK);
            GFX_setTextSize(2);
            GFX_setTextColor(ST77XX_GREEN);
            GFX_setCursor(20, 20);
            GFX_printf("ST7789 OK!");

            GFX_setTextColor(ST77XX_CYAN);
            GFX_setCursor(20, 50);
            GFX_printf("20px safe");

            GFX_flush();
            break;
        }

        sleep_ms(8000);              // Hold each test for 8 seconds
        testNum = (testNum + 1) % 4; // Cycle through 4 tests

        tight_loop_contents();
    }
}
