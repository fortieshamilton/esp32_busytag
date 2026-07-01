// Example TFT_eSPI User_Setup for 1.47" ST7789 (172x320)
// Copy this file into your project and include via TFT_eSPI User_Setup mechanism
// Adjust pin numbers to match your wiring/board.

#define ST7789_DRIVER
#define TFT_WIDTH 172
#define TFT_HEIGHT 320

// SPI pins - change to match your board
#define TFT_MOSI  11
#define TFT_SCLK  13
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4  // or set to -1 and wire to EN/RST on board
#define TFT_BL    12 // backlight control (optional)

// Color order
#define TFT_RGB_ORDER TFT_RGB

// Font etc can be left default

// Uncomment if using HSPI/VSPI default pins on some boards
//#define TFT_MISO  -1

// No further configuration here; if your board already supplies a User_Setup in the core,
// prefer editing that file or using the User_Setup_Select mechanism.
