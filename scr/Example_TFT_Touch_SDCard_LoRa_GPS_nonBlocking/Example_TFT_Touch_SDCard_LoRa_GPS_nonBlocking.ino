#include <TFT_eSPI.h>
#include <SD.h>
#include <XPT2046_Touchscreen.h>

#include <LoRa.h>
//#include <RadioLib.h>

#include <SPI.h>
#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
#include <MicroNMEA.h> //http://librarymanager/All#MicroNMEA


// Pin-Definitionen für TFT (ist aber in der TFT_eSPI Lib "User_Setup - LoRaHAMDisplay.h" und gewählt in der "User_Setup_Select.h" vordefiniert.
#define TFT_MISO          5
#define TFT_MOSI          4
#define TFT_SCLK          6
#define TFT_CS            1   // Chip select control pin
#define TFT_DC            45  // Data Command control pin
#define TFT_RST           7   // Reset pin

// Pin-Definitionen für LoRa, SD und Touchscreen
#define LORA_MOSI         35
#define LORA_MISO         37
#define LORA_SCK          36
#define LORA_CS           34           // LoRa radio chip select (NSS)
#define LORA_RST          2             // LoRa radio reset
#define LORA_IRQ0         3             // DIO0
#define LORA_IRQ1         21            // DIO1
#define LCD_Backlight     15            // Hintergrundbeleuchtung
#define SDCARD_CS         38            // SD-Karte CS
#define XPT2046_CS        17            // Touchscreen CS
#define XPT2046_IRQ       33            // Touchscreen IRQ

#define GPS_SCL           40
#define GPS_SDA           41

#define LED               39

String Call               = "DC0XX";    // Callsign
String CallSSID           = "-11";      // APRS SSID
   
// LoRa Frequenzen:  Simplex ist 433,775 
int LoRaFreqRX            = 433775E3;  // Simplex Frequenz. 
int LoRaFreqTX            = 433775E3;  // Alternativ Relaismode: DB0ARD sendet auf 433,900 / IARU

int LoRaSF= 12;
int LoRaBW= 125E3;   // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. defaults to 125E3
int LoRaPower = 15;

// 1278 = 433; 1276 = 868
//SX1276 radio = new Module(LORA_CS, LORA_IRQ0, LORA_RST, LORA_IRQ1);

#define xMin 429
#define yMin 769
#define xMax 3360
#define yMax 3310



SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

TFT_eSPI tft = TFT_eSPI(); // TFT Objekt
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ); // Touchscreen Objekt

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Wire.begin(GPS_SDA, GPS_SCL);
    
  // Hintergrundbeleuchtung definieren
  pinMode(LED, OUTPUT);
  pinMode(LCD_Backlight, OUTPUT);

  pinMode(LORA_CS, OUTPUT);
  pinMode(LORA_RST, OUTPUT);
  pinMode(SDCARD_CS, OUTPUT);
  pinMode(XPT2046_CS, OUTPUT);
  digitalWrite(LORA_CS, HIGH);
  digitalWrite(LORA_RST, HIGH);
  digitalWrite(SDCARD_CS, HIGH);
  digitalWrite(XPT2046_CS, HIGH);

  // Initialisierung des TFT mit dem definierten SPI
  tft.begin(); // TFT initialisieren
  tft.setRotation(0); // Displayrotation auf 0 setzen
  tft.fillScreen(TFT_BLACK);
  
  // Hintergrundbeleuchtung aktivieren
  digitalWrite(LCD_Backlight, HIGH); // Einschalten
   

  SetupLoRaModule();  
  
  // Touchscreen initialisieren
  ts.begin();
  ts.setRotation(1);


  // SD-Karte initialisieren
  if (!SPIFFS.begin()) {
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.println("SPIFFS Fehler!");
    Serial.println("SPIFFS Fehler!");
  } else {
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.println("SPIFFS bereit!");
    Serial.println("SPIFFS bereit!");
  }


 // SD-Karte initialisieren
 if (!SD.begin(SDCARD_CS)) {
   tft.setTextColor(TFT_RED);
   tft.setTextSize(2);
   tft.println("SD-Karte nicht gefunden!");
   Serial.println("SD-Karte nicht gefunden!");
 } else {
   tft.setTextColor(TFT_GREEN);
   tft.setTextSize(2);
   tft.println("SD-Karte bereit!");
   Serial.println("SD-Karte bereit!");
 }
 

 myGNSS.setPacketCfgPayloadSize(UBX_NAV_SAT_MAX_LEN); // Allocate extra RAM to store the full NAV SAT data

 if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
 {
   tft.setTextColor(TFT_RED);    
   tft.println("GPS Fehler.");
   Serial.println("GPS Fehler.");
   //while (1);
 }else{
   tft.setTextColor(TFT_GREEN);    
   tft.println("GPS bereit.");
   Serial.println("GPS bereit.");
 }

//  myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA); //Set the I2C port to output UBX only (turn off NMEA noise)
 myGNSS.setI2COutput(COM_TYPE_NMEA); //Set the I2C port to output NMEA
 myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
 myGNSS.setProcessNMEAMask(SFE_UBLOX_FILTER_NMEA_ALL); // Make sure the library is passing all NMEA messages to processNMEA

 //myGNSS.setProcessNMEAMask(SFE_UBLOX_FILTER_NMEA_GGA); // Or, we can be kind to MicroNMEA and _only_ pass the GGA messages to it
 
 myGNSS.setNavigationFrequency(1); //Produce one solution per second

   tft.setTextColor(TFT_YELLOW);
   tft.print("Tx: ");
   String message = "<\xFF\x01"+Call+CallSSID+">APRS,TCPIP:=4827.70N/00957.93E@LMS";   // Position im APRS initialisieren. Ersetze die Koordinaten durch deine (schaue auf die Karte von aprs.fi).
   LoRaSendMessage(message);
     
}



void loop() {
  digitalWrite(LED, !digitalRead(LED)); // Toggle LED auf dem LoRaHAM-Board bei jhedem Durchlauf

  if (ts.tirqTouched()) {
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      tft.fillCircle( ((p.y-800) / 5.308) , ((p.x -400)/ 9.159), 10, TFT_RED); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!
      Serial.print("Pressure = ");
      Serial.print(p.z);
      Serial.print(", x = ");
      Serial.print(p.x);
      Serial.print(", y = ");
      Serial.print(p.y);
      delay(30);
      Serial.println();
    }
  }


  if (runEvery(60000)) { // repeat every 5000 millis
   tft.setTextColor(TFT_YELLOW);
   tft.print("Tx: ");

   String message = "<\xFF\x01"+Call+CallSSID+">APRS,TCPIP,WIDE1-1::DC2WA-15 :Dies ist eine Testnachricht ans ❤ Smartphone! Hallo LoRaHAM!";   // send a message
   LoRaSendMessage(message); 
  }

/*
  // LoRa senden (Beispiel)
  digitalWrite(LORA_CS, LOW); // Aktivieren LoRa
  LoRa.beginPacket();
  LoRa.print("Hallo von ESP32-S2!");
  Serial.println("Hallo von ESP32-S2!");
  LoRa.endPacket();
  digitalWrite(LORA_CS, HIGH); // Deaktivieren LoRa
*/

 // delay(1000); // Kurze Verzögerung
}


boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
