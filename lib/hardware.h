/*
 OLED hardware configuration for GMT147SPI-ST7789 project
 Date: 6th Oct 2025
 Rev: 0.1
 Author: Alejandro Moglia
 valves@bartola.co.uk
 Github:
*/

/*
OLED display using ST7789 display
Libraries located on lib/oled/
SPI1 used for OLED display

OLED_RESET on GPIO 6
OLED_SDA on GPIO 7
OLED_DC on GPIO 8
OLED_CS on GPIO 9
OLED_SCK on GPIO 10
*/
#define OLED_RESET_PIN 6
#define OLED_SDA_PIN 11
#define OLED_DC_PIN 8
#define OLED_CS_PIN 9
#define OLED_SCK_PIN 10
#define OLED_SPI_PORT spi1

// BL pin is connected to 3.3V directly on the PCB
