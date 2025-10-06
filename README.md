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

The test application demonstrates basic display functionality:

1. **Initialization:** Configures SPI and initializes the display
2. **Screen Clear:** Fills the screen with black
3. **Text Rendering:** Displays "Hello ST7789!" text
4. **Main Loop:** Enters infinite loop to keep the display active

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

1. **Initial Issue:** Display hung after initialization with "Clearing display..." message
2. **Root Cause:** SPI Mode inconsistency - init used Mode 0, but data transfer functions used Mode 3
3. **Solution:** Changed all SPI operations to Mode 0 (CPOL_0, CPHA_0)
4. **Speed Testing:** 
   - 1 MHz: Works (initial testing)
   - 4 MHz: Works (current baseline) ✅
   - Higher speeds: Not yet tested on breadboard

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
