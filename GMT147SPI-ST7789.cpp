/**
 * @file GMT147SPI-ST7789.cpp
 * @brief Test program and safe area validation for ST7789P3 display driver
 * @author Alejandro Moglia (@bartola-valves)
 * @date 6th Oct 2025
 * @version 0.1 - Working Baseline Configuration
 *
 * HARDWARE CONFIGURATION:
 * - Display: ST7789P3 variant, 1.47", 172×320 pixels, rounded corners
 * - Microcontroller: Raspberry Pi Pico (RP2040)
 * - Interface: SPI1 (4 MHz, Mode 0)
 * - Pin Mapping: RST=GPIO6, DC=GPIO8, CS=GPIO9, SCK=GPIO10, MOSI=GPIO11
 *
 * CRITICAL CONFIGURATION FOR ST7789P3:
 * - SPI Mode 0 (CPOL_0, CPHA_0) - REQUIRED, Mode 3 causes hang
 * - All SPI functions must use consistent Mode 0 configuration
 * - ST7789P3 variant differs from standard ST7789 (Mode 3)
 *
 * SAFE AREA FOR ROUNDED CORNERS:
 * - Verified Margin: 10 pixels on all sides
 * - Safe Area: 152×300 pixels (172-20, 320-20)
 * - Global Constant: SAFE_MARGIN = 10
 * - Determination Method: Empirical visual testing with color-coded borders
 *
 * TEST FUNCTIONS:
 * 1. testBasicText()           - Validates display initialization and text rendering
 * 2. testSafeZone()             - Visual border test to verify safe margins (CRITICAL)
 * 3. testCharacterCapacity()    - Demonstrates character grid (12×18 = 216 chars)
 * 4. testPracticalLayout()      - Shows real-world UI layout example
 *
 * MAIN LOOP:
 * - Cycles through all test functions every 8 seconds
 * - Allows continuous visual verification of display behavior
 * - Serial output provides detailed information for each test
 *
 * ASSUMPTIONS:
 * - Display orientation: 0° (portrait, 172px width × 320px height)
 * - Framebuffer architecture: Full RAM buffer, GFX_flush() updates display
 * - Text size 2 recommended: 12×16 pixels per character (optimal readability)
 * - Color rendering: RGB565 format, may appear different than color names suggest
 *
 * TROUBLESHOOTING:
 * - If display hangs: Check SPI Mode 0 in ALL spi_set_format() calls
 * - If text truncated: Verify using SAFE_MARGIN for all UI elements
 * - If colors wrong: Check display initialization includes ST77XX_INVON command
 *
 * @note This file serves as both a test program and reference implementation
 *       for future projects using ST7789P3 displays with rounded corners.
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
 * @brief SAFE AREA DEFINITION for rounded corner display
 *
 * Physical Display: ST7789P3 1.47" with rounded corners (172×320 pixels)
 *
 * SAFE ZONE (verified through visual testing):
 * - Margin: 10 pixels on ALL sides (top, bottom, left, right)
 * - Safe Area Dimensions: 152×300 pixels
 * - Calculation: (172-20) × (320-20) = 152×300
 * - Coordinate Range: X[10..161], Y[10..309]
 *
 * CHARACTER GRID (Size 2 text: 12×16 pixels):
 * - Characters per row: 12 (152÷12 = 12.67, rounded down to 12)
 * - Rows: 18 (300÷16 = 18.75, rounded down to 18)
 * - Total capacity: 216 characters (12×18)
 *
 * VISUAL TEST RESULTS:
 * - Red border (0px margin): ❌ TRUNCATED by rounded corners
 * - Orange border (10px margin): ✅ PERFECT - "second box from outside", reaches edge perfectly
 * - Yellow border (20px margin): ✅ SAFE - extra conservative margin
 * - Green border (25px margin): ✅ EXTRA SAFE - more margin than needed
 *
 * RECOMMENDATION: Use 10px margin for optimal screen usage
 */
const int SAFE_MARGIN = 10; ///< Safe margin in pixels for rounded corners (verified)

/**
 * @brief Test function to determine character capacity of the display
 * @param textSize Font size multiplier (1 = 6x8 pixels per char, 2 = 12x16, etc.)
 *
 * PURPOSE:
 * This function calculates and displays how many characters fit within the safe
 * display area at a given text size, accounting for rounded corner margins.
 * It fills the screen with numbered rows (R00, R01, etc.) to visually verify
 * the calculation and demonstrate practical character grid layout.
 *
 * METHODOLOGY:
 * 1. Calculate safe area dimensions (screen size minus margins)
 * 2. Determine character dimensions at requested text size
 * 3. Calculate grid: floor(safe_width / char_width) × floor(safe_height / char_height)
 * 4. Draw all rows with labels to demonstrate actual capacity
 * 5. Draw border at SAFE_MARGIN to visualize boundary
 *
 * Default font dimensions: 6 pixels wide × 8 pixels tall (size 1)
 * Actual char dimensions = (6 * textSize) × (8 * textSize)
 *
 * RESULTS for ST7789P3 1.47" display (172×320, 10px margin):
 * - Size 1: 25×37 = 925 chars  (too small, hard to read) ❌
 * - Size 2: 12×18 = 216 chars  (RECOMMENDED - optimal readability) ✅
 * - Size 3: 8×12 = 96 chars    (large, limited capacity) ⚠️
 *
 * VISUAL VERIFICATION:
 * - Orange border drawn at SAFE_MARGIN (10px) to show safe zone
 * - All row labels (R00-R17) should be fully visible within border
 * - Last row should not be cut off by rounded corners
 *
 * @note Uses global SAFE_MARGIN constant (verified through testSafeZone())
 */
void testCharacterCapacity(uint8_t textSize)
{
    // Font characteristics (default built-in font)
    const int charWidth = 6;  // pixels per character (width)
    const int charHeight = 8; // pixels per character (height)
    // Using SAFE_MARGIN = 10px (verified safe zone for rounded corners)

    // Calculate actual dimensions with text size multiplier
    int actualCharWidth = charWidth * textSize;
    int actualCharHeight = charHeight * textSize;

    // Calculate safe display area (accounting for margins on all sides)
    int safeWidth = lcd_width - (SAFE_MARGIN * 2);
    int safeHeight = lcd_height - (SAFE_MARGIN * 2);

    // Calculate how many characters fit in safe area (floor division)
    int charsPerRow = safeWidth / actualCharWidth;
    int numRows = safeHeight / actualCharHeight;

    // Display results on serial
    printf("\n=== Character Capacity Test (Safe Zone) ===\n");
    printf("Display: %dx%d pixels\n", lcd_width, lcd_height);
    printf("Safe zone: %dx%d pixels (%dpx margins - VERIFIED)\n", safeWidth, safeHeight, SAFE_MARGIN);
    printf("Text size: %d\n", textSize);
    printf("Character dimensions: %dx%d pixels\n", actualCharWidth, actualCharHeight);
    printf("Characters per row: %d\n", charsPerRow);
    printf("Number of rows: %d\n", numRows);
    printf("Total characters: %d\n", charsPerRow * numRows);
    printf("================================\n\n");

    // Clear screen
    GFX_fillScreen(ST77XX_BLACK);

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

    // Draw safe zone border AFTER text so it overlays on top
    // Using ORANGE to match the verified 10px safe zone from testSafeZone()
    // This is the "second box from outside" that the user confirmed is perfect
    GFX_drawRect(10, 10, lcd_width - 20, lcd_height - 20, ST77XX_ORANGE);

    GFX_flush();
    printf("Test pattern complete!\n");
    printf("All rows should be fully visible within orange border (10px margin from edges).\n");
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
 * PURPOSE:
 * The 1.47" ST7789P3 display has rounded corners that crop edge content.
 * This function performs EMPIRICAL VISUAL TESTING to determine the optimal
 * safe drawing area by drawing multiple colored borders at different margins.
 *
 * METHODOLOGY:
 * 1. Draw four concentric rectangular borders at different margin values:
 *    - Red (0px):    Edge of screen - WILL BE CROPPED
 *    - Orange (10px): Small margin - TO BE VERIFIED
 *    - Yellow (20px): Medium margin - TO BE VERIFIED
 *    - Green (25px):  Large margin - CONSERVATIVELY SAFE
 *
 * 2. Draw text labels at specific positions to aid identification:
 *    - "Edge" at (15, 0): Near top-left corner (offset right to avoid "E" truncation)
 *    - "10px" at (10, 25): Inside orange border area
 *    - "20px" at (20, 50): Inside yellow border area
 *    - "Safe Zone" at (25, 75): Inside green border area
 *
 * 3. User visually inspects display to identify which border:
 *    - Reaches the edge of visible area without cropping
 *    - Appears as "second box from outside"
 *    - Shows all pixels without rounded corner truncation
 *
 * VERIFICATION RESULTS (6th Oct 2025):
 * - Red border (0px):    ❌ TRUNCATED by rounded corners
 * - Orange border (10px): ✅ PERFECT - "second box from outside", reaches edge perfectly
 * - Yellow border (20px): ✅ SAFE - conservative margin, extra space
 * - Green border (25px):  ✅ EXTRA SAFE - most conservative, reduces usable area
 *
 * OPTIMAL MARGIN: 10 pixels (orange border)
 * - Maximizes usable screen area
 * - No visible cropping on actual hardware
 * - Confirmed through user visual inspection
 *
 * WHY EMPIRICAL TESTING:
 * - Display datasheets don't specify exact corner radius
 * - Manufacturing variations affect actual safe area
 * - Visual confirmation more reliable than theoretical calculations
 * - Device-specific verification required for each display variant
 *
 * COLOR NAMING NOTE:
 * RGB565 colors may appear different than their names suggest:
 * - ST77XX_ORANGE may appear brown/orange
 * - ST77XX_YELLOW may appear yellow/brown
 * Use border POSITION ("second from outside") not just color for identification.
 *
 * @note This function should be run whenever using a new display variant
 *       to verify the safe margin is appropriate for that specific hardware.
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

    // Test text at different positions (separated to avoid overlap)
    GFX_setTextSize(2);

    // Position 1: (15,0) - Edge (shifted right 15px to avoid "E" truncation) - CYAN
    GFX_setTextColor(ST77XX_CYAN);
    GFX_setCursor(15, 0);
    GFX_printf("Edge");

    // Position 2: (10,25) - 10px margin (moved down to avoid overlap) - MAGENTA
    GFX_setTextColor(ST77XX_MAGENTA);
    GFX_setCursor(10, 25);
    GFX_printf("10px");

    // Position 3: (20,50) - 20px margin (testing) - WHITE
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(20, 50);
    GFX_printf("20px Safe?");

    // Position 4: (25,75) - 25px margin (extra safe) - GREEN
    GFX_setTextColor(ST77XX_GREEN);
    GFX_setCursor(25, 75);
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
 * @brief Practical usage test - demonstrates typical UI layout with safe margins
 *
 * PURPOSE:
 * Shows a realistic multi-line text display layout that respects the verified
 * safe margins for rounded corner displays. Demonstrates a common UI pattern:
 * header, body content, and status information.
 *
 * LAYOUT STRUCTURE:
 * - Header: Yellow "ST7789 Test" title at top
 * - Body: Multiple white text lines with system information
 * - Status: Cyan "Status: OK" at bottom
 * - All elements positioned within SAFE_MARGIN boundaries
 *
 * MARGIN USAGE:
 * - Uses global SAFE_MARGIN constant (10px)
 * - Ensures all text starts at (SAFE_MARGIN, y) positions
 * - Provides consistent spacing from display edges
 * - Demonstrates proper safe area utilization
 *
 * LINE SPACING:
 * - Size 2 text: 16 pixels tall
 * - LINE_HEIGHT: 20 pixels (16 + 4 pixel spacing)
 * - Prevents text rows from touching
 *
 * REAL-WORLD APPLICATIONS:
 * - Status displays
 * - Sensor readouts
 * - Menu systems
 * - Information panels
 *
 * @note This serves as a reference implementation for any UI code
 *       that needs to respect rounded corner safe areas.
 */
void testPracticalLayout()
{
    printf("\n=== Practical Layout Test ===\n");
    printf("Simulating typical text display with %dpx margins...\n", SAFE_MARGIN);

    const int LINE_HEIGHT = 20; // Height for size 2 text (16 + 4 spacing)

    GFX_fillScreen(ST77XX_BLACK);
    GFX_setTextSize(2);

    int yPos = SAFE_MARGIN;

    // Title (Header section)
    GFX_setTextColor(ST77XX_YELLOW);
    GFX_setCursor(SAFE_MARGIN, yPos);
    GFX_printf("ST7789 Test");
    yPos += LINE_HEIGHT + 5;

    // Body (Information section)
    GFX_setTextColor(ST77XX_WHITE);
    GFX_setCursor(SAFE_MARGIN, yPos);
    GFX_printf("Display:");
    yPos += LINE_HEIGHT;

    GFX_setCursor(SAFE_MARGIN, yPos);
    GFX_printf("172x320 px");
    yPos += LINE_HEIGHT;

    GFX_setCursor(SAFE_MARGIN, yPos);
    GFX_printf("10px margin"); // Reflects verified SAFE_MARGIN value
    yPos += LINE_HEIGHT + 10;

    // Status indicator (Body content)
    GFX_setTextColor(ST77XX_GREEN);
    GFX_setCursor(SAFE_MARGIN, yPos);
    GFX_printf("Safe Zone!");

    // Footer (Status section)
    GFX_setTextColor(ST77XX_CYAN);
    int footerY = lcd_height - SAFE_MARGIN - 16; // 16 = height of size 2 text
    GFX_setCursor(SAFE_MARGIN, footerY);

    // Calculate usable rows with safe margins
    int usableHeight = lcd_height - (2 * SAFE_MARGIN);
    int usableRows = usableHeight / LINE_HEIGHT;
    GFX_printf("~%d rows", usableRows);

    GFX_flush();

    printf("Safe area dimensions:\n");
    printf("  Width: %d pixels (%d margin each side)\n", lcd_width - (2 * SAFE_MARGIN), SAFE_MARGIN);
    printf("  Height: %d pixels (%d margin top/bottom)\n", lcd_height - (2 * SAFE_MARGIN), SAFE_MARGIN);
    printf("  Usable rows (size 2, 20px spacing): %d\n", usableRows);
    printf("  Characters per row: ~12\n"); // Updated from ~14 to reflect 10px margin
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
    printf("║  VERIFIED RECOMMENDATIONS for 1.47\" ST7789P3:     ║\n");
    printf("║  • Text Size: 2 (12x16 pixels) - OPTIMAL          ║\n");
    printf("║  • Safe Margin: 10 pixels from edges ✅ VERIFIED  ║\n");
    printf("║    (empirically tested with border overlay)       ║\n");
    printf("║  • Red box (0px): Truncated by rounded corners    ║\n");
    printf("║  • Orange box (10px): PERFECT - edges visible     ║\n");
    printf("║  • Usable Area: 152x300 pixels (10px margin)      ║\n");
    printf("║  • Practical Rows: ~15 (with 20px line spacing)   ║\n");
    printf("║  • Characters/Row: ~12 (size 2, 10px margins)     ║\n");
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
