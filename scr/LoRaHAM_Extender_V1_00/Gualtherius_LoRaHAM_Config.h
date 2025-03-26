//const int csPin = 5;         // Old LoRa radio chip select
//const int csPin = 2;           // LoRa radio chip select (NSS)
//const int resetPin = 13;       // LoRa radio reset
//const int irqPin = 35;         // change for your board; must be a hardware interrupt pin

const int csPin         = 34;           // LoRa radio chip select (NSS)
const int resetPin      = 2;       // LoRa radio reset
const int irqPin        = 3;         // change for your board; must be a hardware interrupt pin

// Led am LoRaHAM-Board
//#define LED               9  // Vor Version 2.06
#define LED               39  // Ab Version 2.06
#define LCD_Backlight     15
