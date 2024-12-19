#include <LoRa.h>

//#include <RadioLib.h>

#include <SPI.h>


// Pin-Definitionen für LoRa, SD und Touchscreen
#define LORA_MOSI         35
#define LORA_MISO         37
#define LORA_SCK          36
#define LORA_CS           34           // LoRa radio chip select (NSS)
#define LORA_RST          2             // LoRa radio reset
#define LORA_IRQ0         3             // DIO0
#define LORA_IRQ1         21            // DIO1
#define LCD_Backlight     15            // Hintergrundbeleuchtung


// Vor 206
//#define LED               9

// Ab  206
#define LED               39

//int transmissionState = RADIOLIB_ERR_NONE;

// LoRa Frequenzen:  Simplex ist 433,775 
int LoRaFreqRX            = 433775E3;  // Simplex Frequenz. 
int LoRaFreqTX            = 433775E3;  // Alternativ Relaismode: DB0ARD sendet auf 433,900 / IARU

int LoRaSF= 12;
int LoRaBW= 125E3;   // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. defaults to 125E3
int LoRaPower = 15;

// 1278 = 433; 1276 = 868
//SX1276 radio = new Module(LORA_CS, LORA_IRQ0, LORA_RST, LORA_IRQ1);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED, OUTPUT);

  pinMode(LORA_CS, OUTPUT);
  pinMode(LORA_RST, OUTPUT);

  digitalWrite(LORA_CS, HIGH);
  digitalWrite(LORA_RST, HIGH);

   
  //SPI.begin(LORA_SCK, LORA_MISO, LORA_MISO );
  SetupLoRaModule();  
  

 delay(100);

   String Call               = "DCxYYY";   // Callsign
   String CallSSID           = "-11";      // APRS SSID

   String message = "<\xFF\x01"+Call+">APRS,TCPIP:=4827.70N/00957.63E@LMS Init LoRaHAM ";   // send a message
    
//   String message = "<\xFF\x01"+Call+">APRS,TCPIP:=4827.70N/00957.63E@LMS Init LoRaHAM";   // send a message
  
   
     LoRaSendMessage(message); //Nachricht raus

}



void loop() {

  if (runEvery(15000)) { // repeat every 5000 millis
   String Call               = "DCxYYY";   // Callsign
   String CallSSID           = "-9";      // APRS SSID
   String message = "<\xFF\x01"+Call+">APRS,TCPIP:=4827.70N/00957.63E@LMS Hallo LoRaHAM !";   // send a message
   digitalWrite(LED, HIGH); // Toggle LED auf dem LoRaHAM-Board bei jhedem Durchlauf
   Serial.println("Sende LoRa:");
   Serial.println(message);
   
    LoRaSendMessage(message); //Nachricht raus
  }

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
