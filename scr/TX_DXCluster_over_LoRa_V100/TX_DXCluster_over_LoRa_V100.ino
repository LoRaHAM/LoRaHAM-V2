// DX-Cluster over LoRa mit LoRaHAM
// https://github.com/LoRaHAM
// https://loraham.de/shop

#include <WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <TFT_eSPI.h>

// WLAN-Zugangsdaten
const char* ssid = "SSID";
const char* password = "passcode";

// Login-Name für DX-Cluster
const char* dx_login = "DB0xyz";



// Telnet-Server (DX-Cluster)
//const char* telnet_host = "srv02.oevsv.at";
const char* telnet_host = "dxc.hamserve.uk";
const uint16_t telnet_port = 7300;

WiFiClient telnetClient;
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

// LoRa-Pin-Definitionen (aus deinem Beispiel)
#define LORA_MOSI     35
#define LORA_MISO     37
#define LORA_SCK      36
#define LORA_CS       34
#define LORA_RST      2
#define LORA_IRQ0     3
#define LORA_IRQ1     21
#define LCD_Backlight 15
#define LED           39  // LED für Statusanzeigen (z. B. auf dem LoRaHAM-Board)

// LoRa-Parameter 
int LoRaFreqRX = 433900E3;    // 433,775 MHz (Simplex) für Lokal, 433,900 MHz bei Relais (DB0ARD)
int LoRaFreqTX = 433900E3;    // 433,775 MHz (Simplex)

int LoRaSF = 10;              // Spreading Factor
int LoRaBW = 125E3;           // Bandbreite in Hz
int LoRaPower = 17;           // Sendeleistung in dBm

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  delay(2000);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW); 

  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LCD_Backlight, LOW); 
  
  tft.init();
  tft.setRotation(3);  
  //tft.invertDisplay(1);                   // Invertiere das Display, falls du eine andere Variante benutzt und die Farben invertiert dargestellt werden.
    
  tft.fillScreen(TFT_BLACK);

  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LCD_Backlight, HIGH); 
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("ESP32 DX-Cluster zu LoRa"); 
    
  Serial.println("ESP32 DX-Cluster zu LoRa");

  // WLAN verbinden 
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WLAN...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWLAN verbunden.");

  delay(2000);
  
  Serial.println("\nVerbinde zu DX-Cluster.");
  tft.println("\nVerbinde zu DX-Cluster.");
  
  // Mit DX-Cluster verbinden
  if (telnetClient.connect(telnet_host, telnet_port)) {
    Serial.println("Mit DX-Cluster verbunden.");
    tft.println("Mit DX-Cluster verbunden.");
  } else {
    Serial.println("Verbindung zu DX-Cluster fehlgeschlagen!");
    tft.println("Verbindung zu DX-Cluster fehlgeschlagen!");
    
    Serial.println("\nReset.");
    tft.println("\nReset.");

    delay(2000);
    ESP.restart();    
    while (true);
  }

  // Warten auf Login-Aufforderung
  while (!telnetClient.available()) {
    delay(100);
  }
  String loginPrompt = telnetClient.readStringUntil('\n');
  Serial.println("Login-Prompt: " + loginPrompt);

  // Login senden
  telnetClient.println(dx_login);
  Serial.println("Login gesendet: " + String(dx_login));
  tft.println("Login gesendet: " + String(dx_login));

  // LoRa initialisieren
  SetupLoRaModule();
  Serial.println("");
}

//String allowedChar(String Message) {
//  String result = "";
//  String allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789öäüÖÄÜ!\"§$%&/()=?,.;:-_#+* ";
//
//  for (unsigned int i = 0; i < Message.length(); i++) {
//    char c = Message.charAt(i);
//    if (allowed.indexOf(c) >= 0) {
//      result += c;
//    }
//  }
//
//  return result;
//}


String allowedChar(String Message) {
  String result = "";

  for (unsigned int i = 0; i < Message.length(); i++) {
    char c = Message.charAt(i);
    // Nur druckbare ASCII-Zeichen (32 bis 126) und erweiterte UTF-8-Zeichen >= 128
    if ((c >= 32 && c <= 126) || (unsigned char)c >= 128) {
      result += c;
    }
  }

  return result;
}



void loop() {
  // DX-Cluster-Daten empfangen
  if (telnetClient.connected() && telnetClient.available()) {
    String dxMessage = telnetClient.readStringUntil('\n');
    dxMessage = allowedChar(dxMessage);
    dxMessage.trim();
    Serial.println("" + dxMessage);
    tft.println(dxMessage);
    int CursorPos = tft.getCursorY();
    if (CursorPos > 300)     tft.setCursor(0, 0);    // Set cursor near top left corner of screen
 

    // LoRa-Nachricht senden
    LoRaSendMessage(dxMessage);
    delay(250);
  }

  // Verbindung prüfen
  if (!telnetClient.connected()) {
    Serial.println("DX-Cluster-Verbindung verloren. Neustart...");
    ESP.restart();
  }
}

// --- Funktionen für LoRa ---

void SetupLoRaModule() {
  // LoRa initialisieren
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ0);
  
  if (!LoRa.begin(LoRaFreqRX)) {
    Serial.println("LoRa Initialisierung fehlgeschlagen!");
  } else {
    Serial.println("LoRa bereit!");
  }

  Serial.println("LoRa Konfig.");

  LoRa.setFrequency(LoRaFreqRX);          // Frequenz setzen
  LoRa.setSpreadingFactor(LoRaSF);        // SF setzen
  LoRa.setSignalBandwidth(LoRaBW);        // Bandbreite setzen
  LoRa.setCodingRate4(5);                 // CodingRate setzen
  LoRa.setPreambleLength(8);              // Preamble
  LoRa.setSyncWord(0x12);                 // SyncWord
  LoRa.enableCrc();                       // CRC einschalten
  LoRa.enableInvertIQ();
  LoRa.disableInvertIQ();
  //LoRa.setGain(6);                      // Gain optional
  LoRa.setTxPower(LoRaPower);             // Sendeleistung

  //LoRa.onTxDone(onTxDone);

  Serial.println("LoRa initialisiert.");
}

void onTxDone() {
  Serial.println("LoRa TxDone");
  digitalWrite(LED, LOW);  // LED ausschalten nach Senden
}

void LoRaSendMessage(String outgoing) {
  digitalWrite(LED, HIGH); 
  LoRa.setFrequency(LoRaFreqTX);   // Frequenz für Senden
  LoRa.beginPacket();
  LoRa.print(outgoing);
  LoRa.endPacket();            // true = async / non-blocking mode
  digitalWrite(LED, LOW);  
  delay(100); 
}
