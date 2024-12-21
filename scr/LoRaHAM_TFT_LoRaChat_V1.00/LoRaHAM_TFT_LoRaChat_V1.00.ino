/*
  
  This code is designed for Gualtherius LoRaHAM V2. Pinout and PCB for 1 Watt of freedom.

  here comes some cool text in future.

  IDE    : Arduino 1.8.19
  Lib    : arduino-LoRa 0.7.2 by sandeepmistry              https://github.com/sandeepmistry/arduino-LoRa
  ESP32  : x.x.x                                            https://dl.espressif.com/dl/package_esp32_index.json
  Board  : ESP32S2 Dev Module
  Target : Gualtherius LoRaHAM ESP32 V2

  created 21.12.2022
  by Alexander Walter

  based on LoRa from Tom Igoe

  https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
  https://github.com/me-no-dev/EspExceptionDecoder

  LoRa-APRS Paketaufbau:
  DCxYY-10>APRS,TCPIP:=4827.70N/00957.65E@LMS LoRaHAM Message Service Test

  Korrekt formatiert:
  DCxYY-10>APRS,TCPIP,WIDE1-1::DCyZZ-15 :Dies ist eine Testnachricht ans ❤ Smartphone
  DCxYY-10>APRS,TCPIP::DCyZZ-15 :Sonderzeichencheck ❤️🥰
  DCxYY-10>APRS,TCPIP::DCyZZ    :Rufzeichencheck ❤
                               ^-- Rufzeichen muss 9 Zeichen sein. Rest füllen!
  CALLSIGN>APRS,TCPIP::CALLSIGN :test message                             
  
  Funktionierende Positionsmeldung:
  DCxYY-10>APRS,TCPIP:=4827.72N/00957.97E@LMS
  DCxYY-10>APRS,TCPIP::DKxYY    :@LMS:Guten Abend{1
  
  Changelog:
  21.12.2022 Init Release
  
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <TFT_eSPI.h>

#include "Gualtherius_LoRaHAM_Config.h"


const String Call = "DCxYYY";   // Callsign



int LoRaFreqTX= 433775E3;  // 433.775 MHz verwenden alle LoRa-APRS Tracker zum Senden.
int LoRaFreqRX= 433775E3;  // 433.775 MHz um Tracker zu empfangen. 433.900 um LoRa-Relaisstationen zu empfangen.
//int LoRaFreqRX= 433900E3;  // 433.775 MHz um Tracker zu empfangen. 433.900 um LoRa-Relaisstationen zu empfangen.



int LoRaSF= 12;
int LoRaBW= 125E3;   // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. defaults to 125E3
int LoRaPower = 17;


word msgCount = 0;            // count of outgoing messages
int interval = 600000;          // interval between sends
long lastSendTime = 0;        // time of last packet send

int buttonState = 0;

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

void setup() {
  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LCD_Backlight, LOW); 

  Serial.begin(115200);                   // initialize serial
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println();
  
  //while (!Serial);
  pinMode(0, INPUT);

  tft.init();
  tft.setRotation(0);  
  tft.fillScreen(TFT_BLACK);

  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LCD_Backlight, HIGH); 

  tft.setTextSize(2);
  tft.setCursor(10, 10);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("Tx: ");
  
  tft.setCursor(10, 240);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("Rx: ");
     

   
  SetupLoRaModule();

}



void loop() {
  buttonState = digitalRead(0);
  if (buttonState == LOW){
    Serial.println("GPIO 0 pressed...");
    Serial.println("Message: RESET");
    sendMessage("RESET_");
    delay(10);
  }

  
  onReceive(LoRa.parsePacket());
  serialLoop();
  delay(10);
}
