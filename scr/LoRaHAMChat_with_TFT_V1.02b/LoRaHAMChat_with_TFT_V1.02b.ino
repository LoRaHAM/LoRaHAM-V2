// LoRaChat mit LoRaHAM
// https://github.com/LoRaHAM
// https://loraham.de/shop

#include <WiFi.h>
#include <WebServer.h>
#include <LoRa.h>        // https://github.com/sandeepmistry/arduino-LoRa
#include <SPIFFS.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <TFT_eSPI.h>

#define TFT_LoRaHAM_BLUE 0x07FF
#define LCD_Backlight     15
#define LED               39  // Ab Version 2.06

// WLAN-Daten
String ssid = "";
String password = "";

unsigned long previousMillis = 0;
unsigned long interval = 30000;

// LoRa Pins (hier müssen je nach Hardware die Pins angepasst werden)
#define SS_PIN    34
#define RST_PIN   2
#define DIO0_PIN  3

// LoRa Parameter
int loraSendFrequency = 433775000;  // Frequenz 433.775 MHz
//int loraReceiveFrequency = 433775000;  // Empfangsfrequenz 433.775 MHz
int loraReceiveFrequency = 433900000;  // Empfangsfrequenz 433.900 MHz für Relaisc
int loraBW = 125E3;          // Bandbreite 125 kHz
int loraSF = 12;             // Spreizung SF12
int loraTXPower = 17;        // Sendeleistung 17 dBm

//#define LORA_FREQUENCY 433775000  // Frequenz 433.775 MHz
//#define LORA_BW 125E3          // Bandbreite 125 kHz
//#define LORA_CR 5              // FEC-Codierung 4/5
//#define LORA_SF 12             // Spreizung SF12

// Webserver auf Port 80
WebServer server(80);

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;


TFT_eSPI tft = TFT_eSPI(); /* TFT instance */


// Nutzername des aktuellen Benutzers
String currentUser = "";


String filterString(const String &input) {
    String output = "";
 
    for (int i = 0; i < input.length(); i++) {
        char c = input.charAt(i);

              
        // Überprüfen, ob das Zeichen alphanumerisch ist
        if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == ' ')  || (c == '!')  || (c == '?') ) {
            output += c; // Zeichen hinzufügen, wenn es gültig ist
        }
    }
    return output; // Rückgabe des gefilterten Strings
}





// Route zum Senden einer Nachricht
void handleSendMessage() {
  String msg = server.arg("msg");
  String user = server.arg("user");

  String filteredString = filterString(msg);
  
  if (msg.length() > 0 && user.length() > 0) {
    String message = "<div class='message message-right'>" + user + ": " + filteredString + "</div>";
    saveMessage(message); // Speichert die Nachricht

    msg = "<\xFF\x01" +currentUser+">APRS,WIDE1-1:"+ msg;
    // Nachricht über LoRa senden
    Serial.print("Sende    ");
    Serial.print(loraSendFrequency);
    Serial.print(" : ");
    Serial.print(msg);

    tft.setCursor(10, 310);    // Set cursor near top left corner of screen
    tft.setTextColor(TFT_LoRaHAM_BLUE, TFT_BLACK);
    tft.print(msg);
    tft.println("          ");
    tft.fillCircle( 310 , 10, 10, TFT_GREEN); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!

   
    LoRa.setFrequency(loraSendFrequency);
    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();
    LoRa.setFrequency(loraReceiveFrequency);
  
    tft.fillCircle( 310 , 10, 10, TFT_BLACK); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!

    
    Serial.print(" ");
    Serial.print(loraReceiveFrequency);
    Serial.println("");
  }

  server.send(200, "text/plain", "Message received");
}

// Nachrichten über LoRa empfangen
void onReceive(int packetSize) {
  if (packetSize) {
    String incomingMessage = "";
    while (LoRa.available()) {
      incomingMessage += (char)LoRa.read();
    }
    Serial.print("Empfange ");
    Serial.print(loraReceiveFrequency);
    Serial.print(" : ");
    Serial.print(incomingMessage);
    Serial.print(" ");
    Serial.print(loraSendFrequency);
    Serial.println("");

    String filteredString = filterString(incomingMessage);
  
    saveMessage("<div class='message message-left'>" + filteredString + "</div>");
    
    tft.setCursor(10, 110);    // Set cursor near top left corner of screen
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print(incomingMessage);
    tft.println("          ");    
  }
}

// Funktion zum Speichern von Nachrichten im SPIFFS
void saveMessage(String message) {
  File file = SPIFFS.open("/chat_history.html", FILE_APPEND);
  if (file) {
    file.println(message);
    file.close();
  } else {
    Serial.println("Fehler beim Speichern der Nachricht");
  }
}

// Funktion zum Laden des Chatverlaufs aus SPIFFS
String loadChatHistory() {
  String chatHistory = "";
  File file = SPIFFS.open("/chat_history.html", FILE_READ);
  if (file) {
    while (file.available()) {
      chatHistory += (char)file.read();
    }
    file.close();
  } else {
    Serial.println("Fehler beim Laden des Chatverlaufs");
  }
  return chatHistory;
}

// Funktion zum Speichern der Konfiguration (im Textformat)
void saveConfig() {
  File configFile = SPIFFS.open("/config.txt", FILE_WRITE);
  if (configFile) {
    configFile.println("sendFrequency=" + String(loraSendFrequency));
    configFile.println("receiveFrequency=" + String(loraReceiveFrequency));
    configFile.println("bw=" + String(loraBW));
    configFile.println("sf=" + String(loraSF));
    configFile.println("txPower=" + String(loraTXPower));
    configFile.println("username=" + String(currentUser));
    configFile.close();
  } else {
    Serial.println("Fehler beim Speichern der Konfiguration");
  }
}

// Funktion zum Laden der Konfiguration (aus Textdatei)
void loadConfig() {
  File configFile = SPIFFS.open("/config.txt", FILE_READ);
  if (configFile) {
    String line;
    while (configFile.available()) {
      line = configFile.readStringUntil('\n');
      if (line.startsWith("sendFrequency=")) {
        loraSendFrequency = line.substring(14).toInt();
      } else if (line.startsWith("receiveFrequency=")) {
        loraReceiveFrequency = line.substring(17).toInt();
      } else if (line.startsWith("bw=")) {
        loraBW = line.substring(3).toInt();
      } else if (line.startsWith("sf=")) {
        loraSF = line.substring(3).toInt();
      } else if (line.startsWith("txPower=")) {
        loraTXPower = line.substring(8).toInt();
      } else if (line.startsWith("username=")) {
        currentUser = line.substring(9);
      }
    }
    configFile.close();
    Serial.println(loraSendFrequency);
    Serial.println(loraReceiveFrequency);
    Serial.println(loraBW);
    Serial.println(loraSF);
    Serial.println(loraTXPower);
    Serial.println(currentUser);
    
  } else {
    Serial.println("Fehler beim Laden der Konfiguration");
  }
}


// HTML-Seite, die im Browser angezeigt wird
String generateChatPage() {
  String html = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>LoRaHAM Chat</title><style>";
  
  html += "body { font-family: 'Arial', sans-serif; margin: 0; padding: 0; background-color: #e5ddd5; display: flex; flex-direction: column; height: 100vh; }";
  html += ".chat-container { flex-grow: 1; overflow-y: auto; padding: 10px; }";
  html += ".message { padding: 10px; border-radius: 20px; margin: 5px 0; display: block; max-width: 75%; word-wrap: break-word; }";
  html += ".message-left { background-color: #fff; align-self: flex-start; }";
  html += ".message-right { background-color: #dcf8c6; align-self: flex-end; }";
  html += ".input-container { display: flex; padding: 10px; background-color: #fff; border-top: 1px solid #ddd; }";
  html += ".input-container input { flex-grow: 1; padding: 10px; font-size: 16px; border: 1px solid #ddd; border-radius: 20px; }";
  html += ".input-container button { padding: 10px 20px; background-color: #25d366; color: white; border: none; border-radius: 20px; cursor: pointer; }";
  html += ".input-container button:hover { background-color: #128c7e; }";
  html += "h1 { color: #075e54; text-align: center; font-size: 24px; margin-top: 10px; }";
  html += "strong { color: #075e54; font-weight: bold; }";
  html += "</style></head><body>";

  // Name-Eingabe-Formular
  html += "<div id='nameForm' style='display: none; text-align: center; margin-top: 50px;'>";
  html += "<h2>Gib deinen Namen ein:</h2>";
  html += "<input type='text' id='username' placeholder='Dein Name' autofocus><br><br>";
  html += "<button onclick='setName()'>Bestätigen</button>";
  html += "</div>";

  // Chat-Bereich
  html += "<div class='chat-container' id='chatContainer' style='display: none;'>" + loadChatHistory() + "</div>";
  
  // Eingabefeld für Nachricht und Name
  html += "<div class='input-container' id='inputContainer' style='display: none;'>";
  html += "<input type='text' id='message' placeholder='Nachricht eingeben...' autofocus><br>";
  html += "<button onclick='sendMessage()'>Senden</button>";
  html += "</div>";

  // JavaScript für die Funktionalität
  html += "<script>";
  
  // Funktion zum Speichern des Namens und Anzeigen des Chat-Bereichs
  html += "function setName() {";
  html += "  var username = document.getElementById('username').value;";
  html += "  if (username.trim() !== '') {";
  html += "    localStorage.setItem('username', username);"; // Speichert den Namen im localStorage
  html += "    document.getElementById('nameForm').style.display = 'none';";
  html += "    document.getElementById('chatContainer').style.display = 'block';";
  html += "    document.getElementById('inputContainer').style.display = 'flex';";
  html += "    checkNewMessages();";  // Neue Nachrichten laden
  html += "  }";
  html += "}";

  // Funktion zum Senden der Nachricht
  html += "function sendMessage() {";
  html += "  var message = document.getElementById('message').value;";
  html += "  var username = localStorage.getItem('username');"; // Holt den gespeicherten Namen
  html += "  if (message.trim() !== '') {";
  html += "    var xhr = new XMLHttpRequest();";
  html += "    xhr.open('GET', '/send?msg=' + message + '&user=' + username, true);";
  html += "    xhr.send();";
  html += "    document.getElementById('message').value = '';";  // Clear the input
  html += "  }";
  html += "}";
  html += "document.getElementById('message').addEventListener('keypress', function(event) {";
  html += "  if (event.key === 'Enter') {";
  html += "    event.preventDefault();";  // Prevent line break
  html += "    sendMessage();";
  html += "  }";
  html += "});";

  // Funktion zum Abrufen neuer Nachrichten
  html += "function checkNewMessages() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/checkNewMessages', true);";
  html += "  xhr.onload = function() {";
  html += "    if (xhr.status === 200) {";
  html += "      var newMessages = xhr.responseText;";
  html += "      if (newMessages) {";
  html += "        document.getElementById('chatContainer').innerHTML = newMessages;";
  html += "        document.getElementById('chatContainer').scrollTop = document.getElementById('chatContainer').scrollHeight;";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  
  // Überprüft, ob der Name im localStorage vorhanden ist
  html += "window.onload = function() {";
  html += "  var username = localStorage.getItem('username');";
  html += "  if (username) {";
  html += "    document.getElementById('nameForm').style.display = 'none';";
  html += "    document.getElementById('chatContainer').style.display = 'block';";
  html += "    document.getElementById('inputContainer').style.display = 'flex';";
  html += "  } else {";
  html += "    document.getElementById('nameForm').style.display = 'block';";
  html += "  }";
  html += "};";
  
  html += "setInterval(checkNewMessages, 1000);";  // Polling every 1 second
  html += "</script>";
  
  html += "</body></html>";
  
  return html;
}

// HTML-Seite für die Konfiguration
String generateConfigPage() {
  String html = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>LoRa Konfiguration</title><style>";
  html += "body { font-family: 'Arial', sans-serif; margin: 0; padding: 0; background-color: #e5ddd5; display: flex; flex-direction: column; height: 100vh; }";
  html += "h1 { color: #075e54; text-align: center; font-size: 24px; margin-top: 10px; }";
  html += "form { padding: 20px; text-align: center; }";
  html += "input, select { margin: 10px; padding: 10px; font-size: 16px; }";
  html += "button { background-color: #25d366; color: white; padding: 10px 20px; border: none; cursor: pointer; }";
  html += "button:hover { background-color: #128c7e; }";
  html += "</style></head><body>";

//  html += "<h1>LoRa Konfiguration</h1>";
  html += "<form id='configForm' method='get' action='/saveConfig'>";


  // Hier kommt das JavaScript, um den Nutzernamen in Großbuchstaben umzuwandeln
  html += "<script>";
  html += "function toUpperCase(input) {";
  html += "  input.value = input.value.toUpperCase();";
  html += "}";
  html += "</script>";

  html += "</head><body>";

  html += "<h1>LoRa Konfiguration</h1>";
  html += "<form id='configForm' method='get' action='/saveConfig'>";
  
//  
//  // Nutzernamen Eingabefeld
//  html += "<label for='username'>Nutzername (max. 15 Zeichen, nur Großbuchstaben, Zahlen und '-'): </label><br>";
//  html += "<input type='text' id='username' name='username' maxlength='15' pattern='[A-Z0-9-]+' required><br>";

  // Frequenz Einstellungen
  html += "<label for='sendFrequency'>Sende-Frequenz:</label><br>";
  html += "<select name='sendFrequency' id='sendFrequency'>";
  html += "<option value='433775000' " + String(loraSendFrequency == 433775000 ? "selected" : "") + ">433.775 MHz</option>";
  html += "<option value='433900000' " + String(loraSendFrequency == 433900000 ? "selected" : "") + ">433.900 MHz</option>";
  html += "</select><br>";

  html += "<label for='receiveFrequency'>Empfangs-Frequenz:</label><br>";
  html += "<select name='receiveFrequency' id='receiveFrequency'>";
  html += "<option value='433775000' " + String(loraReceiveFrequency == 433775000 ? "selected" : "") + ">433.775 MHz</option>";
  html += "<option value='433900000' " + String(loraReceiveFrequency == 433900000 ? "selected" : "") + ">433.900 MHz</option>";
  html += "</select><br>";

  html += "<label for='bw'>Bandbreite:</label><br>";
  html += "<select name='bw' id='bw'>";
  html += "<option value='125000' " + String(loraBW == 125E3 ? "selected" : "") + ">125 kHz</option>";
  html += "<option value='250000' " + String(loraBW == 250E3 ? "selected" : "") + ">250 kHz</option>";
  html += "</select><br>";

  html += "<label for='sf'>Spreading Factor:</label><br>";
  html += "<select name='sf' id='sf'>";
  for (int i = 7; i <= 12; i++) {
    html += "<option value='" + String(i) + "' " + String(loraSF == i ? "selected" : "") + ">SF" + String(i) + "</option>";
  }
  html += "</select><br>";

  html += "<label for='txPower'>Sendeleistung (dBm):</label><br>";
  html += "<select name='txPower' id='txPower'>";
  for (int i = 0; i <= 17; i++) {
    html += "<option value='" + String(i) + "' " + String(loraTXPower == i ? "selected" : "") + ">" + String(i) + " dBm</option>";
  }
  html += "</select><br><br>";
  
  // Nutzernamen Eingabefeld mit oninput-Attribut für automatische Großschreibung
  html += "<label for='username'>Nutzername (max. 15 Zeichen, nur Großbuchstaben, Zahlen und '-'): </label><br>";
  html += "<input type='text' id='username' name='username' maxlength='15' pattern='[A-Z0-9-]+' required oninput='toUpperCase(this)'><br>";

  html += "<button type='submit'>Speichern</button>";
  html += "</form>";

  html += "</body></html>";
  return html;
}

// Route zum Abrufen der HTML-Seite
void handleRoot() {
  server.send(200, "text/html", generateChatPage());
}

// Route zum Abrufen der Konfigurationsseite
void handleConfig() {
  server.send(200, "text/html", generateConfigPage());
}

// Route zum Speichern der Konfiguration
void handleSaveConfig() {
  currentUser = server.arg("username");
  loraSendFrequency = server.arg("sendFrequency").toInt();
  loraReceiveFrequency = server.arg("receiveFrequency").toInt();
  loraBW = server.arg("bw").toInt();
  loraSF = server.arg("sf").toInt();
  loraTXPower = server.arg("txPower").toInt();

  // Speichern der neuen Konfiguration
  saveConfig();

  // Übernehmen der Änderungen in die LoRa-Einstellungen
  LoRa.setFrequency(loraSendFrequency);
  LoRa.setSignalBandwidth(loraBW);
  LoRa.setSpreadingFactor(loraSF);
  LoRa.setTxPower(loraTXPower);
  server.send(200, "text/html", generateChatPage());

}




// Route zum Überprüfen neuer Nachrichten
void handleCheckNewMessages() {
  server.send(200, "text/plain", loadChatHistory());
}

void initTFT(){
  tft.init();
  tft.setRotation(0);  
  tft.invertDisplay(1);                   // Invertiere das Display, falls du eine andere Variante benutzt und die Farben invertiert dargestellt werden.
  
  tft.fillScreen(TFT_BLACK);

  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LCD_Backlight, HIGH); 

  tft.setTextSize(2);

  tft.setCursor(30, 20);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  //tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_LoRaHAM_BLUE, TFT_BLACK);
  tft.print("LoRaHAM ");
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("CHAT");
  //tft.setTextDatum(TL_DATUM); // Change the text datum to the top left corner
  
  
  tft.setCursor(10, 90);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("Rx: ");
  
  tft.setCursor(10, 290);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("Tx: ");
  
  
  for (int i=0; i <= 4; i++){
      digitalWrite(LED, HIGH); 
      delay(100);
      digitalWrite(LED, LOW); 
      delay(100);
  }
}

void setup() {

  pinMode(LED, OUTPUT);    
  pinMode(LCD_Backlight, OUTPUT);    
  digitalWrite(LED, LOW); 
  digitalWrite(LCD_Backlight, LOW); 

  initTFT();
  
  // Starte den seriellen Monitor
  Serial.begin(115200);
  delay(3000);

  // SPIFFS initialisieren
  if (!SPIFFS.begin()) {
    Serial.println("Fehler beim Mounten des Dateisystems");
    Serial.println("Formatiere fröhlich vor mich hin...");
    bool formatted = SPIFFS.format();
    Serial.println("So jetzt! Erledigt! Neustart...");
    ESP.restart();
    return;
  }


    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    //res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //res = wm.autoConnect("LoRaChat_Config_AP",""); // password protected ap

//    if(!res) {
//        Serial.println("Failed to connect");
//        // ESP.restart();
//    } 
//    else {
//        //if you get here you have connected to the WiFi    
//        Serial.println("connected...yeey :)");
//    }



  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP("LoRaHAM_Chat", "")) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  tft.setCursor(30, 35);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_LoRaHAM_BLUE, TFT_BLACK);
  tft.print("My IP: ");
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(myIP);   

      
  pinMode(0, INPUT);
  loadConfig();
  
  // LoRa initialisieren
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  if (!LoRa.begin(loraSendFrequency)) {
    Serial.println("LoRa Initialisierung fehlgeschlagen!");
    while (1);
  }

  Serial.println("LoRa Initialisierung in Ordnung!");
  
  LoRa.setFrequency(loraReceiveFrequency);
  LoRa.setSpreadingFactor(loraSF);
  LoRa.setSignalBandwidth(loraBW);
  LoRa.setPreambleLength(8); // Supported values are between 6 and 65535. defaults to 8
  LoRa.setSyncWord(0x12); // byte value to use as the sync word, defaults to 0x12
  //LoRa.enableCrc();
  LoRa.disableCrc();
  LoRa.disableInvertIQ();  // Enable or disable Invert the LoRa I and Q signals, by default a invertIQ is not used.
  LoRa.setTxPower(loraTXPower);
  LoRa.receive();  // Setzt den ESP32 auf Empfang

  // Webserver-Routen definieren
  server.on("/", HTTP_GET, handleRoot);
  server.on("/send", HTTP_GET, handleSendMessage);
  server.on("/checkNewMessages", HTTP_GET, handleCheckNewMessages);

  server.on("/lora", HTTP_GET, handleConfig);
  server.on("/saveConfig", HTTP_GET, handleSaveConfig);

  // Webserver starten
  server.begin();
}

void loop() {

  if (digitalRead(0) == LOW) {
        Serial.println("Erasing Config, restarting");
        delay(3000); // reset delay 
        wm.resetSettings();
        ESP.restart();
  }
  
  // Webserver ständig abfragen
  server.handleClient();
  
  // LoRa-Nachrichten empfangen
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      onReceive(packetSize);
    }

  if (runEvery(1000)) { // repeat every 5000 millis
    digitalWrite(LED, HIGH); 
    delay(100);
    digitalWrite(LED, LOW); 
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
