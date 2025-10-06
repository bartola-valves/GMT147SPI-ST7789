# GMT147SPI-ST7789

ST7789P3 TFT LCD Display Driver for Raspberry Pi Pico

## Overview

This project provides a working driver and test application for the **ST7789P3** TFT LCD display (172x320 pixels, 1.47") on the Raspberry Pi Pico microcontroller. The driver is specifically configured and tested for the **ST7789P3 variant**, which has different SPI requirements compared to standard ST7789 displays.

**Author:** Alejandro Moglia (@bartola-valves)  
**Email:** valves@bartola.co.uk  
**Date:** 6th October 2025  
**Version:** 0.1 - Working Baseline Configuration

## Hardware

### Display Specifications
- **Controller:** ST7789P3 (P3 variant)
- **Resolution:** 172x320 pixels
- **Size:** 1.47 inches
- **Interface:** SPI
- **Color Depth:** 16-bit (RGB565)

### Connections

The display connects to the Raspberry Pi Pico via SPI1 with the following pin mapping:

| Function | Pico GPIO | Display Pin | Description |
|----------|-----------|-------------|-------------|
| RESET    | GPIO 6    | RST         | Hardware reset (active low) |
| MOSI     | GPIO 11   | SDA         | SPI data out (Master Out Slave In) |
| DC       | GPIO 8    | DC          | Data/Command select |
| CS       | GPIO 9    | CS          | Chip select (active low) |
| SCK      | GPIO 10   | SCK         | SPI clock |
| VCC      | 3.3V      | VCC         | Power supply |
| GND      | GND       | GND         | Ground |
| BL       | 3.3V      | BL          | Backlight (direct connection) |

**Note:** The backlight (BL) pin is connected directly to 3.3V in this configuration.

### Wiring Diagram

```
Raspberry Pi Pico          ST7789P3 Display
┌─────────────────┐       ┌──────────────┐
│                 │       │              │
│  GPIO 6  ──────────────→ RST          │
│  GPIO 8  ──────────────→ DC           │
│  GPIO 9  ──────────────→ CS           │
│  GPIO 10 ──────────────→ SCK          │
│  GPIO 11 ──────────────→ SDA (MOSI)   │
│  3.3V    ──────────────→ VCC          │
│  3.3V    ──────────────→ BL           │
│  GND     ──────────────→ GND          │
│                 │       │              │
└─────────────────┘       └──────────────┘
```

## Critical Configuration for ST7789P3

### ⚠️ Important: ST7789P3 vs Standard ST7789

The **ST7789P3** variant requires specific SPI configuration that differs from standard ST7789 displays:

#### SPI Mode
- **Required:** SPI Mode 0 (CPOL=0, CPHA=0)
- **Previous (non-working):** SPI Mode 3 (CPOL=1, CPHA=1)
- **Impact:** Using Mode 3 causes the display to hang during large data transfers (framebuffer writes)

#### SPI Speed
- **Tested Working:** 4 MHz (breadboard configuration)
- **Recommended for PCB:** Up to 62.5 MHz
- **Current Setting:** 4 MHz for stable breadboard operation

### Configuration Summary

```cpp
// Working configuration in lib/oled/st7789.cpp
spi_init(st7789_spi, 4000000);                                    // 4 MHz
spi_set_format(st7789_spi, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); // Mode 0
```

## Project Structure

```
GMT147SPI-ST7789/
├── GMT147SPI-ST7789.cpp      # Main test application
├── CMakeLists.txt             # Build configuration
├── pico_sdk_import.cmake      # Pico SDK import script
├── README.md                  # This file
├── lib/
│   ├── hardware.h             # Hardware pin definitions
│   └── oled/
│       ├── st7789.cpp         # ST7789P3 driver implementation
│       ├── st7789.h           # ST7789P3 driver header
│       ├── gfx.cpp            # Graphics library
│       ├── gfx.h              # Graphics library header
│       ├── gfxfont.h          # Font definitions
│       └── font.h             # Default font data
└── build/                     # Build output directory
```

## Building the Project

### Prerequisites

- Raspberry Pi Pico SDK installed
- CMake (version 3.13 or higher)
- ARM GCC toolchain
- Visual Studio Code with Raspberry Pi Pico extension (recommended)

### Build Steps

#### Using VS Code (Recommended)

1. Open the project folder in VS Code
2. Use the "Compile Project" task from the Raspberry Pi Pico extension
3. The binary will be generated in `build/GMT147SPI-ST7789.uf2`

#### Using Command Line

```bash
mkdir build
cd build
cmake ..
make
```

### Flashing the Binary

#### Method 1: USB Bootloader
1. Hold the BOOTSEL button on the Pico while connecting USB
2. Copy `build/GMT147SPI-ST7789.uf2` to the mounted drive

#### Method 2: SWD Debugger (Recommended for Development)
Use the "Flash" task in VS Code with a debug probe connected

## Usage

The test application demonstrates basic display functionality and includes empirical validation of the safe display area:

### Test Functions (Main Loop Cycle - 8 seconds each)

1. **Basic Text Test (`testBasicText()`):** 
   - Validates display initialization and text rendering
   - Shows "Hello Pico!" at size 2 and "Size 1 text" at size 1
   - Confirms colors and basic drawing work correctly

2. **Safe Zone Test (`testSafeZone()`)**
   - **Critical for rounded corner displays**
   - Draws four colored borders at different margins (0px, 10px, 20px, 25px)
   - Includes text labels to identify margin sizes
   - Purpose: Visual verification of safe drawing area
   - **User must observe which border reaches the edge without truncation**

3. **Character Capacity Test (`testCharacterCapacity()`)**
   - Displays maximum number of characters that fit in safe area
   - Shows row labels (R00-R17) for 18 rows at size 2 text
   - Draws orange border at 10px margin (verified safe zone)
   - Demonstrates 12×18 character grid = 216 character capacity

4. **Practical Layout Test (`testPracticalLayout()`)**
   - Shows real-world UI example using safe margins
   - Demonstrates header/body/footer layout pattern
   - Uses 10px margins consistently

### Logic and Assumptions

#### Display Geometry Assumptions

1. **Physical Resolution:** 172×320 pixels (hardware specification)
2. **Rounded Corners:** Significant corner radius requires empirical testing
3. **Rotation:** 0° orientation (portrait mode with 172px width, 320px height)
4. **No Calibration Data:** Display doesn't report its safe area - must be determined visually

#### Safe Margin Determination Logic

**Problem:** Rounded corner displays don't have a standard safe area specification.

**Solution:** Empirical visual testing with multiple border overlays

**Method:**
```cpp
// Draw borders at increasing margins
GFX_drawRect(0, 0, lcd_width, lcd_height, ST77XX_RED);           // 0px - edge
GFX_drawRect(10, 10, lcd_width - 20, lcd_height - 20, ST77XX_ORANGE); // 10px
GFX_drawRect(20, 20, lcd_width - 40, lcd_height - 40, ST77XX_YELLOW); // 20px
GFX_drawRect(25, 25, lcd_width - 50, lcd_height - 50, ST77XX_GREEN);  // 25px
```

**Key Insight:** The user identifies which border "reaches the edge perfectly without cropping" by visual inspection. In this case, the **orange border (10px margin)** was confirmed as optimal.

#### GFX_drawRect() Function Behavior

**Function Signature:** `GFX_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)`

**Parameters:**
- `x, y`: Top-left corner coordinates (starting position)
- `w, h`: Width and height (dimensions, NOT endpoint coordinates)
- Rectangle occupies pixels from (x,y) to (x+w-1, y+h-1)

**Example:**
```cpp
// 10px margin border
GFX_drawRect(10, 10, lcd_width - 20, lcd_height - 20, ST77XX_ORANGE);
// Draws from (10, 10) to (161, 309) on a 172×320 display
// Width: 172 - 20 = 152 pixels
// Height: 320 - 20 = 300 pixels
```

#### Character Capacity Calculation

**Default Font (Size 1):** 6×8 pixels per character
**Size 2 Multiplier:** 12×16 pixels per character (6×2, 8×2)

**Calculation for 10px margin:**
```cpp
Safe Width:  172 - (10 * 2) = 152 pixels
Safe Height: 320 - (10 * 2) = 300 pixels

Characters per row: 152 / 12 = 12.67 → 12 (floor)
Rows: 300 / 16 = 18.75 → 18 (floor)
Total capacity: 12 × 18 = 216 characters
```

**Why floor division:** Partial characters would be truncated/unreadable, so only complete character cells are counted.

#### Design Decisions

1. **SAFE_MARGIN as Global Constant**
   - Single source of truth for all UI elements
   - Easy to adjust if display variant differs
   - Located at top of GMT147SPI-ST7789.cpp

2. **Color-Coded Testing**
   - Red (0px): Clearly shows cropping problem
   - Orange (10px): Optimal - verified by user inspection
   - Yellow (20px): Conservative fallback
   - Green (25px): Maximum safety, reduces usable space
   - **Avoids ambiguity through distinct visual markers**

3. **Text Size 2 as Standard**
   - Size 1 (6×8): Too small for comfortable reading on 1.47" display
   - Size 2 (12×16): **Recommended** - good balance of readability and capacity
   - Size 3 (18×24): Large but limited space (only 8×12 = 96 chars)

4. **8-Second Loop Timing**
   - Allows user to observe each test comfortably
   - Automatic cycling enables continuous verification
   - Can be modified in `main()` loop

### Display Safe Area (Rounded Corners)

The ST7789P3 1.47" display has **rounded corners** that require a safe margin to ensure content is not cropped.

#### Safe Zone Specification (VERIFIED THROUGH VISUAL TESTING)
- **Margin:** 10 pixels on ALL sides (top, bottom, left, right)
- **Safe Area Dimensions:** 152×300 pixels
- **Calculation:** (172-20) × (320-20) = 152×300
- **Coordinate Range:** X[10..161], Y[10..309]
- **Global Constant:** `SAFE_MARGIN = 10` (defined in GMT147SPI-ST7789.cpp)

#### Visual Test Results (Border Overlay Method)

The project includes a visual test function (`testSafeZone()`) that draws colored borders at different margins:

| Border Color | Margin | Width×Height | Result | Notes |
|--------------|--------|--------------|--------|-------|
| **Red** | 0px | 172×320 | ❌ TRUNCATED | Outer pixels cut off by rounded corners |
| **Orange** | 10px | 152×300 | ✅ **PERFECT** | "Second box from outside" - reaches edge perfectly |
| **Yellow** | 20px | 132×280 | ✅ SAFE | Conservative, extra margin |
| **Green** | 25px | 130×270 | ✅ EXTRA SAFE | Most conservative, wasted screen space |

**Key Finding:** The orange border at 10px margin is the **optimal balance** - it reaches the visible edge without any cropping. This was confirmed by visual inspection where the orange box appears to "reach the edge of the rounded corners perfectly."

#### Testing Methodology

The safe zone was determined through empirical visual testing:

1. **Multiple Border Overlay:** Draw concentric rectangles at 0px, 10px, 20px, and 25px margins
2. **Color Differentiation:** Each margin uses a distinct color for easy identification
3. **Visual Confirmation:** User inspects which border reaches the edge without truncation
4. **"Second from Outside" Rule:** The orange border (10px) was identified as the perfect fit

This approach is more reliable than theoretical calculations because:
- It accounts for actual display manufacturing variations
- It reveals the true visible area after rounded corner cropping
- It's device-specific and verified on actual hardware

#### Character Grid (Size 2 Text: 12×16 pixels)

When using the recommended text size 2 within the safe area:
- **Characters per row:** 12 (152÷12 = 12.67, rounded down)
- **Number of rows:** 18 (300÷16 = 18.75, rounded down)
- **Total capacity:** 216 characters (12×18)

**Recommendation:** Always use **10px margin** (`SAFE_MARGIN = 10`) from edges for optimal screen utilization on this rounded corner display. This provides the maximum usable area while ensuring no content is cropped.

### Expected Serial Output

```
SPI peripheral set to spi1
ST7789 display pins configured
SPI peripheral configured for LCD
Initializing display...
Display initialized with 172x320 resolution
Display rotation set to 0 degrees
Creating framebuffer...
Framebuffer created successfully
Clearing display...
Screen cleared!
Drawing text...
Text drawn!
Entering main loop...
```

## API Reference

### Display Initialization

```cpp
// Set the SPI peripheral
LCD_setSPIperiph(spi1);

// Configure GPIO pins (DC, CS, RST, SCK, MOSI)
LCD_setPins(OLED_DC_PIN, OLED_CS_PIN, OLED_RESET_PIN, OLED_SCK_PIN, OLED_SDA_PIN);

// Initialize display with dimensions
LCD_initDisplay(lcd_width, lcd_height);

// Set rotation (0=0°, 1=90°, 2=180°, 3=270°)
LCD_setRotation(0);
```

### Graphics Functions

```cpp
// Create framebuffer
GFX_createFramebuf(lcd_width, lcd_height);

// Fill entire screen with color
GFX_fillScreen(ST77XX_BLACK);  // or ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE, etc.

// Set text properties
GFX_setTextSize(2);
GFX_setTextColor(ST77XX_WHITE);
GFX_setCursor(x, y);
GFX_printf("Hello World!");

// Draw shapes
GFX_drawPixel(x, y, color);
GFX_drawLine(x0, y0, x1, y1, color);
GFX_drawRect(x, y, w, h, color);
GFX_fillRect(x, y, w, h, color);
GFX_drawCircle(x, y, radius, color);
GFX_fillCircle(x, y, radius, color);

// Update display (send framebuffer to LCD)
GFX_flush();
```

### Color Definitions

```cpp
ST77XX_BLACK    // 0x0000
ST77XX_WHITE    // 0xFFFF
ST77XX_RED      // 0xF800
ST77XX_GREEN    // 0x07E0
ST77XX_BLUE     // 0x001F
ST77XX_CYAN     // 0x07FF
ST77XX_MAGENTA  // 0xF81F
ST77XX_YELLOW   // 0xFFE0
ST77XX_ORANGE   // 0xFC00
```

## Troubleshooting

### Display shows nothing / blank screen

1. **Verify wiring:** Ensure all connections match the pin mapping above
2. **Check MOSI pin:** Must be connected to **GPIO 11** (common mistake: GPIO 7)
3. **Verify power:** 3.3V and GND connections, backlight connected to 3.3V
4. **Check SPI mode:** Must be Mode 0 (CPOL_0, CPHA_0) for ST7789P3

### Display hangs during initialization

1. **SPI Mode:** Ensure all `spi_set_format()` calls use Mode 0
2. **SPI Speed:** Try reducing to 1 MHz if 4 MHz doesn't work
3. **Wiring:** Check for loose connections, especially on breadboards

### Text appears upside down

Change the rotation value in `LCD_setRotation()`:
- `0` = 0° (normal)
- `1` = 90° (portrait, rotated right)
- `2` = 180° (upside down)
- `3` = 270° (portrait, rotated left)

### Colors appear inverted

The display initialization includes the `ST77XX_INVON` command which is required for ST7789P3. This is correctly configured in the baseline.

## Development Notes

### Testing History (6th Oct 2025)

#### Initial Display Hang Issue

1. **Initial Issue:** Display hung after initialization with "Clearing display..." message
2. **Root Cause:** SPI Mode inconsistency
   - `initSPI()` configured Mode 0 (CPOL_0, CPHA_0)
   - But `ST7789_WriteCommand()` and `ST7789_WriteData()` used Mode 3 (CPOL_1, CPHA_1)
   - Caused hang during large data transfers (framebuffer writes)
3. **Solution:** Changed all SPI operations to Mode 0 consistently
   - **Critical:** ST7789P3 variant requires Mode 0 (differs from some ST7789 docs)
   - All SPI functions now use: `spi_set_format(spi, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST)`

#### Pin Mapping Resolution

- **Initial Confusion:** MOSI pin varied in examples (GPIO 7 vs GPIO 11)
- **Resolution:** Verified with hardware that MOSI must be on **GPIO 11** (SPI1 TX)
- **Reason:** spi1 peripheral uses fixed hardware pins:
  - SCK: GPIO 10
  - MOSI (TX): GPIO 11
  - MISO (RX): GPIO 12 (not used for this display)

#### Safe Margin Determination Process

1. **Initial Assumption:** Tried 20px margin (common for rounded displays)
2. **Visual Testing Method:**
   - Drew concentric borders at 0px, 10px, 20px, 25px
   - Used distinct colors (red, orange, yellow, green)
   - Asked user to identify which border "reaches the edge perfectly"
3. **Key Finding:** User reported orange border (10px) as "second box from outside" that "reaches edge perfectly"
4. **Confusion Resolution:** 
   - User initially said "brown box is perfect"
   - Confusion arose because multiple colors appear brownish on display
   - Clarification revealed: "second box from outside" = orange at 10px
5. **Final Verification:** Changed from 20px to 10px margin, confirmed visually
6. **Result:** SAFE_MARGIN = 10 pixels (optimal utilization without cropping)

#### Speed Testing

- 1 MHz: Works (initial conservative testing)
- 4 MHz: Works (current baseline) ✅
- Higher speeds: Not yet tested on breadboard
- Note: Breadboard capacitance limits practical speeds

### Key Insights for Future Development

#### 1. ST7789P3 Variant Specificity

The "P3" variant has undocumented quirks:
- **Must use SPI Mode 0** (not Mode 3 like some ST7789 displays)
- Verify with oscilloscope if implementing on different hardware
- Don't assume ST7789 documentation applies directly to ST7789P3

#### 2. Empirical Testing is Critical

For rounded corner displays:
- **Don't trust theoretical calculations** - actual corner radius varies
- **Visual overlay testing** is most reliable method
- Test on actual hardware, not datasheets
- Manufacturing tolerances affect safe area

#### 3. Color Naming Pitfalls

RGB565 color rendering varies by display:
- `ST77XX_YELLOW` may appear orange/brown
- `ST77XX_ORANGE` may appear brown/red
- Use multiple distinct colors for testing
- Verify by position ("second box from outside") not just color name

#### 4. GFX Function Parameter Gotcha

`GFX_drawRect(x, y, w, h, color)` uses WIDTH and HEIGHT, not x2/y2:
- Common error: Using endpoint coordinates instead of dimensions
- To get border with margin M: `drawRect(M, M, lcd_width - 2*M, lcd_height - 2*M, color)`
- Subtract `2*M` (margin on both sides), NOT just `M`

#### 5. Framebuffer Architecture

Current implementation:
- Full framebuffer stored in RAM (172×320×2 bytes = 110KB)
- `GFX_flush()` required to update display
- Drawing operations modify RAM, not display directly
- Advantage: Flicker-free updates, complex animations possible
- Disadvantage: High RAM usage (careful on memory-constrained projects)

#### 6. Text Sizing Trade-offs

| Size | Pixels/Char | Chars/Row | Rows | Total | Readability |
|------|-------------|-----------|------|-------|-------------|
| 1    | 6×8         | 25        | 37   | 925   | ❌ Too small |
| 2    | 12×16       | 12        | 18   | 216   | ✅ **Optimal** |
| 3    | 18×24       | 8         | 12   | 96    | ⚠️ Very large |

Size 2 recommended for 1.47" displays.

### Performance Optimization

For production PCB implementations, the SPI speed can be increased:

```cpp
// In lib/oled/st7789.cpp - initSPI()
spi_init(st7789_spi, 62500000); // 62.5 MHz - maximum for ST7789
```

**Note:** Higher speeds may require shorter traces and proper PCB layout practices.

## Known Issues

- None currently reported with baseline configuration

## Future Enhancements

- [ ] DMA support for faster framebuffer transfers
- [ ] Hardware SPI optimization
- [ ] Additional graphics primitives
- [ ] Touch screen support (if applicable)
- [ ] Power management / sleep modes

## License

This project is provided as-is for educational and development purposes.

## Credits

- Original ST7789 driver adapted for ST7789P3 variant
- Graphics library based on Adafruit GFX concepts
- Raspberry Pi Pico SDK by Raspberry Pi Foundation

## Contact

**Alejandro Moglia**  
GitHub: [@bartola-valves](https://github.com/bartola-valves)  
Email: valves@bartola.co.uk  
Project: [GMT147SPI-ST7789](https://github.com/bartola-valves/GMT147SPI-ST7789)

---

**Last Updated:** 6th October 2025  
**Status:** ✅ Working Baseline Configuration
