//                            USER DEFINED SETTINGS
//   Set driver type, fonts to be loaded, pins used and SPI control method etc
//
//   See the User_Setup_Select.h file if you wish to be able to define multiple
//   setups and then easily select which setup file is used by the compiler.
//
//   If this file is edited correctly then all the library example sketches should
//   run without the need to make any more changes for a particular hardware setup!
//   Note that some sketches are designed for a particular TFT pixel width/height

// User defined information reported by "Read_User_Setup" test & diagnostics example
#define USER_SETUP_INFO "User_Setup"
#define ILI9341_2_DRIVER

  
#define TFT_MISO 37 // Matching T_DO
#define TFT_MOSI 35 // Matching T_DIN
#define TFT_SCLK 36 // Matching T_CLK
#define TFT_CS 34 // Chip select control pin
#define TFT_DC 7 // Data Command control pin
#define TFT_RST 6 // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
#define TFT_BL -1 // LED back-light (only for ST7789 with backlight control pin)

#define TOUCH_MOSI 32
#define TOUCH_MISO 39
#define TOUCH_SCK 25
#define TOUCH_CS 33
#define TOUCH_IRQ 36

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT


#define SPI_FREQUENCY 40000000 // Maximum to use SPIFFS
#define SPI_READ_FREQUENCY 16000000
// The XPT2046 requires a lower SPI clock rate of 2.5MHz so we define that here:
#define SPI_TOUCH_FREQUENCY 1000000

#define USE_HSPI_PORT


//#define USE_HSPI_PORT
