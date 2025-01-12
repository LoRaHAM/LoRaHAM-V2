String serialMessage = "";  
String message = "";        
long SendTime = 0;        // time of last packet send

void serialLoop() {
  while (Serial.available() > 0) {
    byte read = Serial.read();

    if (read == '\n') {
      Serial.println("UART in: " + serialMessage);
      Serial.print(" Sending...");

     
      SendTime=millis();
      sendMessage(serialMessage);


       Serial.print("OK");
       Serial.print(" Time: ");
       Serial.print(millis() - SendTime);
       Serial.println("ms");
      
      serialMessage="";
      continue;
    }
   
    if (read > 0 && serialMessage.length() < 250) {
      serialMessage += (char) read;
    }
    
  }
}
