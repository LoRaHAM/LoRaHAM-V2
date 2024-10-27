

void SetupLoRaModule() {

  // LoRa initialisieren LoRa-Lib
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ0);
  if (!LoRa.begin(LoRaFreqRX)) { // Frequenz auf 433,775 MHz setzen
    //tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.println("LoRa Init fehlgeschlagen!");
    Serial.println("LoRa Initialisierung fehlgeschlagen!");
  } else {
    //tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.println("LoRa bereit!");
    Serial.println("LoRa bereit!");
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
  
  LoRa.onTxDone(onTxDone);
  
}

void RestoreLoRaModule() {

  // LoRa re-initialisieren LoRa-Lib
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ0);
  if (!LoRa.begin(LoRaFreqRX)) { // Frequenz auf 433,775 MHz setzen
    digitalWrite(LORA_CS, LOW);
    digitalWrite(LORA_RST, LOW);
    RestoreLoRaModule();
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

}

void onTxDone() {
  
  Serial.println("LoRa TxDone");
  tft.println("LoRa TxDone");
  RestoreLoRaModule();  

}

// LoRa-Lib
void LoRaSendMessage(String outgoing) {
  LoRa.setFrequency(LoRaFreqTX);   // Change the frequency of the radio.
 
  LoRa.beginPacket();              // start packet
  LoRa.print(outgoing);            // add payload
  LoRa.endPacket(true);            // finish packet and send it // true = async / non-blocking mode

 
} 

volatile bool transmittedFlag = false;

void setFlag(void) {
  // we sent a packet, set the flag
  transmittedFlag = true;
}
