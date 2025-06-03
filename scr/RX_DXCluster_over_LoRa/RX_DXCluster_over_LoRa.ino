// RX DX-Cluster over LoRa mit LoRaHAM
// https://github.com/LoRaHAM
// https://loraham.de/shop


#include <SPI.h>
#include <LoRa.h>
#include <TFT_eSPI.h>


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

#define TFT_LoRaHAM_BLUE 0x07FF

// LoRa-Parameter 
int LoRaFreqRX = 433900E3;    // 433,775 MHz (Simplex) für Lokal, 433,900 MHz bei Relais (DB0ARD)
int LoRaFreqTX = 433900E3;    // 433,775 MHz (Simplex)

int LoRaSF = 10;              // Spreading Factor
int LoRaBW = 125E3;           // Bandbreite in Hz
int LoRaPower = 0;           // Sendeleistung in dBm
String LastMessage = "";
int LastCursorPos = 0;

void setup() {
  Serial.begin(115200);
  delay(3000);

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

  tft.setTextColor(TFT_LoRaHAM_BLUE, TFT_BLACK);
  tft.print("LoRaHAM "); 
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("DX-Cluster Display"); 

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.println(" - connect USB for Terminal output!"); 
    
  Serial.println("ESP32 RX DX-Cluster zu LoRa");


  // LoRa initialisieren
  SetupLoRaModule();
  Serial.println("");
}

String TrimmString(String Nachricht) {
  const int ZielLaenge = 78;
  int aktuelleLaenge = Nachricht.length();

  if (aktuelleLaenge < ZielLaenge) {
    int differenz = ZielLaenge - aktuelleLaenge;
    for (int i = 0; i < differenz; i++) {
      Nachricht += ' ';
    }
  }

  return Nachricht;
}


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

  // LoRa-Nachrichten empfangen
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      onReceive(packetSize);
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

// Nachrichten über LoRa empfangen
void onReceive(int packetSize) {
  if (packetSize) {
    String incomingMessage = "";
    while (LoRa.available()) {
      incomingMessage += (char)LoRa.read();
    }

      String dxMessage = incomingMessage;
      dxMessage = allowedChar(dxMessage);
      dxMessage.trim();
      dxMessage = TrimmString(dxMessage);
      Serial.println("" + dxMessage);

      int CursorPos = tft.getCursorY();  
      if (CursorPos > 300)  CursorPos = 8;    // Set cursor near top left corner of screen

      tft.setCursor(0, LastCursorPos);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);         
      tft.println(LastMessage);

      tft.setCursor(0, CursorPos);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);         
      tft.println(dxMessage);
      delay(100);
      tft.setCursor(0, CursorPos);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);         
      tft.println(dxMessage);

      LastMessage = dxMessage;
      LastCursorPos = CursorPos;

  }
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
