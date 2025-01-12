
void SetupLoRaModule() {
   
  Serial.println("LoRa Duplex - Set spreading factor");
  LoRa.setPins(csPin, resetPin, irqPin); // set CS, reset, IRQ pin

  
  if (!LoRa.begin(LoRaFreqRX)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.setFrequency(LoRaFreqRX);   // Change the frequency of the radio.
  LoRa.setSpreadingFactor(LoRaSF);           // ranges from 6-12,default 7 see API docs
  LoRa.setSignalBandwidth(LoRaBW);           // Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3. defaults to 125E3
  LoRa.setCodingRate4(5);  // Supported values are between 5 and 8,defaults to 5
  LoRa.setPreambleLength(8); // Supported values are between 6 and 65535. defaults to 8
  LoRa.setSyncWord(0x12); // byte value to use as the sync word, defaults to 0x12
  LoRa.enableCrc();
  // LoRa.disableCrc();  // Enable or disable CRC usage, by default a CRC is not used.
  LoRa.enableInvertIQ();
  LoRa.disableInvertIQ();  // Enable or disable Invert the LoRa I and Q signals, by default a invertIQ is not used.
  //LoRa.setGain(6);  // Supported values are between 0 and 6. If gain is 0, AGC will be enabled and LNA gain will not be used. Else if gain is from 1 to 6, AGC will be disabled and LNA gain will be used.
  LoRa.setTxPower(LoRaPower); // TX power in dB, defaults to 17
  Serial.println("LoRa init succeeded.");
}


void sendMessage(String outgoing) {
  LoRa.setFrequency(LoRaFreqTX);   // Change the frequency of the radio.

  tft.setTextSize(2);
  tft.setCursor(10, 50);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("Tx: ");
  tft.setCursor(0, 65);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
//  tft.fillRect(0, 65, 150, 480, TFT_BLACK);
  tft.print(outgoing);
  tft.println("          ");

  tft.fillCircle( 310 , 10, 10, TFT_GREEN); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!
  LoRa.beginPacket();                   // start packet
  LoRa.print("<\xFF\x01"+outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  tft.fillCircle( 310 , 10, 10, TFT_DARKGREY); // Zeichne einen roten Punkt. Diese Koordinaten vom Touch sind nicht kalibriert und dienen nur zum Test ob der Touch funktioniert!

  LoRa.setFrequency(LoRaFreqRX);   // Change the frequency of the radio.
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  Serial.println("Incoming LoRa.");
  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }


  Serial.println(" Message: " + incoming);



   Serial.print("RSSI: " + String(LoRa.packetRssi()));
   Serial.print(" Snr: " + String(LoRa.packetSnr()));
   Serial.println(" freqErr: " + String(LoRa.packetFrequencyError()));

  tft.setCursor(10, 150);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print("Rx: ");
  tft.setCursor(0, 165);    // Set cursor near top left corner of screen
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
//  tft.fillRect(0, 260, 245, 480, TFT_BLACK);
  
  tft.print(incoming);
  tft.println("          ");
}
