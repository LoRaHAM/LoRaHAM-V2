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
  DCxYY-10>APRS,TCPIP:=4827.71N/00957.81E@LMS
  DCxYY-10>APRS,TCPIP::DKxYY    :@LMS:Guten Abend{1

  Experimental Coordinates:
  48.461971
  9.965320
  
  Changelog:
  21.12.2022 Init Release
  
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <TFT_eSPI.h>

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
SFE_UBLOX_GNSS myGNSS;

#include "Gualtherius_LoRaHAM_Config.h"
long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.


const String Call     = "DC2WA";   // Callsign
const String CallSSID = "-10";   // Callsign

bool GPSFix = false;

//int LoRaFreqTX= 433900E3;  // 433.775 MHz verwenden alle LoRa-APRS Tracker zum Senden.
int LoRaFreqTX= 433775E3;  // 433.775 MHz verwenden alle LoRa-APRS Tracker zum Senden.
//int LoRaFreqRX= 433775E3;  // 433.775 MHz um Tracker zu empfangen. 433.900 um LoRa-Relaisstationen zu empfangen.
int LoRaFreqRX= 433900E3;  // 433.775 MHz um Tracker zu empfangen. 433.900 um LoRa-Relaisstationen zu empfangen.



int LoRaSF= 12;
int LoRaBW= 125E3;   // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. defaults to 125E3
int LoRaPower = 13;


String APRSDegree         = "";
String Degree             = "";
String Minutes            = "";
String Seconds            = "";

String APRSPos            = "";

word msgCount = 0;            // count of outgoing messages
int interval = 600000;          // interval between sends
long lastSendTime = 0;        // time of last packet send

int buttonState = 0;

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

void setup() {
  pinMode(LED, OUTPUT);    
  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LED, LOW); 
  digitalWrite(LCD_Backlight, LOW); 

  Serial.begin(115200);                   // initialize serial
  Wire.begin(GPS_SDA,GPS_SCL);
  
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

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(260, 5);    // Set cursor near top left corner of screen
    tft.print("G");
    
  tft.fillCircle( 310 , 10, 10, TFT_DARKGREY); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!


  for (int i=0; i <= 4; i++){
      digitalWrite(LED, HIGH); 
      delay(100);
      digitalWrite(LED, LOW); 
      delay(100);
  }
  

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. "));
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(260, 5);    // Set cursor near top left corner of screen
    tft.print("G");
   
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
  

   
  SetupLoRaModule();

}



void loop() {
  buttonState = digitalRead(0);
  if (buttonState == LOW){
    Serial.println("GPIO 0 pressed...");
    Serial.println("Message: Boot0");
    //sendMessage(Call+CallSSID+"APRS,TCPIP:=4827.71N/00957.81E@LMS LoRaHAM Message Service");
    if (GPSFix == true) {
        sendMessage(APRSPos);
      } else {
      sendMessage(Call+CallSSID+">APRS,WIDE2-2:@LMS LoRaHAM Message Service");
      }
      
    Serial.println("OK!");
    delay(10);
  }

  if (APRSrunEvery(60E3)) { // repeat every 5000 millis
    if (GPSFix == true) {
      sendMessage(APRSPos);
    } else {
      //sendMessage(Call+CallSSID+">APRS,WIDE2-2:LoRaHAM  :@LMS LoRaHAM has no GPS-Fix");
    }

  }


  if (runEvery(1000)) { // repeat every 5000 millis
    tft.setCursor(280, 5);    // Set cursor near top left corner of screen
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("@");
    //tft.fillCircle( 290 , 10, 10, TFT_GREEN); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!
    digitalWrite(LED, HIGH); 
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setCursor(280, 5);    // Set cursor near top left corner of screen
    delay(100);
    tft.print("@");
    //tft.fillCircle( 290 , 10, 10, TFT_DARKGREY); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!
    digitalWrite(LED, LOW); 
  }
  
  onReceive(LoRa.parsePacket());
  serialLoop();

  if (millis() - lastTime > 2000)
  {
    lastTime = millis(); //Update the timer
        byte fixType = myGNSS.getFixType();
    Serial.print(F(" Fix: "));
    if(fixType == 0) Serial.print(F("No fix"));
    else if(fixType == 1) Serial.print(F("Dead reckoning"));
    else if(fixType == 2) Serial.print(F("2D"));
    else if(fixType == 3) Serial.print(F("3D"));
    else if(fixType == 4) Serial.print(F("GNSS + Dead reckoning"));
    else if(fixType == 5) Serial.print(F("Time only"));
    Serial.println();

    if(fixType > 0){
      GPSFix = true;
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(260, 5);    // Set cursor near top left corner of screen
      tft.print("G");
    
      long latitude = myGNSS.getLatitude();
      Serial.print(F("Lat: "));
      Serial.print(latitude);
  
      long longitude = myGNSS.getLongitude();
      Serial.print(F(" Long: "));
      Serial.print(longitude);
      Serial.print(F(" (degrees * 10^-7)"));
  
      long altitude = myGNSS.getAltitude();
      Serial.print(F(" Alt: "));
      Serial.print(altitude);
      Serial.print(F(" (mm)"));
  
      byte SIV = myGNSS.getSIV();
      Serial.print(F(" SIV: "));
      Serial.print(SIV);
      Serial.println();

      byte RTK = myGNSS.getCarrierSolutionType();
      Serial.print(" RTK: ");
      Serial.print(RTK);
      if (RTK == 0) Serial.print(F(" (No solution)"));
      else if (RTK == 1) Serial.print(F(" (High precision floating fix)"));
      else if (RTK == 2) Serial.print(F(" (High precision fix)"));      

      Serial.println();
      Serial.println("APRS Coordinates");
      DegreesToDegMinSec(latitude);
      //APRSPos = Call+CallSSID+">APRS,WIDE1-1:="+Degree+Minutes+"."+Seconds.substring(0,2)+"N/";
      APRSPos = Call+CallSSID+">APRS,WIDE1-1:="+ APRSDegree +"N/";
      DegreesToDegMinSec(longitude);
      if (Degree <= "9") {
        Serial.println("Degree <= 9 ");
        APRSPos +="00"+ APRSDegree +"E@LMS LoRaHAM";;
       }else{
        Serial.println("Degree >= 10 ");
        APRSPos +="0"+ APRSDegree +"E@LMS LoRaHAM";
       }

      //APRSPos += 
  
    } else {
      tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft.setCursor(260, 5);    // Set cursor near top left corner of screen
      tft.print("G");
      GPSFix = false;
    }
      Serial.println();

  if (myGNSS.getNAVSAT()) // Poll the latest NAV SAT data
  {
    Serial.println();

    // See u-blox_structs.h for the full definition of UBX_NAV_SAT_data_t
    Serial.print(F("New NAV SAT data received. It contains data for "));
    Serial.print(myGNSS.packetUBXNAVSAT->data.header.numSvs);
    if (myGNSS.packetUBXNAVSAT->data.header.numSvs == 1)
        Serial.println(F(" SV."));
    else
        Serial.println(F(" SVs."));

    // Just for giggles, print the signal strength for each SV as a barchart
    for (uint16_t block = 0; block < myGNSS.packetUBXNAVSAT->data.header.numSvs; block++) // For each SV
    {
        switch (myGNSS.packetUBXNAVSAT->data.blocks[block].gnssId) // Print the GNSS ID
        {
        case 0:
            Serial.print(F("GPS     "));
        break;
        case 1:
            Serial.print(F("SBAS    "));
        break;
        case 2:
            Serial.print(F("Galileo "));
        break;
        case 3:
            Serial.print(F("BeiDou  "));
        break;
        case 4:
            Serial.print(F("IMES    "));
        break;
        case 5:
            Serial.print(F("QZSS    "));
        break;
        case 6:
            Serial.print(F("GLONASS "));
        break;
        default:
            Serial.print(F("UNKNOWN "));
        break;      
        }
        
        Serial.print(myGNSS.packetUBXNAVSAT->data.blocks[block].svId); // Print the SV ID
        
        if (myGNSS.packetUBXNAVSAT->data.blocks[block].svId < 10) Serial.print(F("   "));
        else if (myGNSS.packetUBXNAVSAT->data.blocks[block].svId < 100) Serial.print(F("  "));
        else Serial.print(F(" "));

        // Print the signal strength as a bar chart
        for (uint8_t cno = 0; cno < myGNSS.packetUBXNAVSAT->data.blocks[block].cno; cno++)
        Serial.print(F("="));

        Serial.println();
    }
  }
     
  }  
  delay(10);
}

void DegreesToDegMinSec(float x)
{
  x= x/10000000;
  int deg=x;
  float minutesRemainder = abs(x - deg) * 60;
  int arcMinutes = minutesRemainder;
  float arcSeconds = (minutesRemainder - arcMinutes) * 60;


  
  Serial.print(deg);Serial.print("*");
  Serial.print(arcMinutes);Serial.print("'");
  Serial.print(arcSeconds,4);Serial.print('"');
  Serial.println();
  Degree=deg;
  Minutes=arcMinutes;
  Seconds=arcSeconds;
  APRSDegree = deg;
  APRSDegree += minutesRemainder;
  
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

boolean APRSrunEvery(unsigned long APRSinterval)
{
  static unsigned long APRSpreviousMillis = 0;
  unsigned long APRScurrentMillis = millis();
  if (APRScurrentMillis - APRSpreviousMillis >= APRSinterval)
  {
    APRSpreviousMillis = APRScurrentMillis;
    return true;
  }
  return false;
}
