void setup() {
  // Start the Serial1 at 9600 baud for debugging output.
  Serial.begin(9600);

  // Start the Serial2 at 9600 baud for communication with the Mega.
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("ESP32 Debug Sketch Initialized");
}

void loop() {
  // If data is available on Serial2 (from the Mega), read it and print it to Serial1.
  while (Serial2.available() > 0) {
    char inChar = (char)Serial2.read();
    Serial.print(inChar);
  }

  // If you also want to relay anything received from Serial1 back to Serial2, uncomment the following lines:
  /*
  while (Serial1.available() > 0) {
    char inChar = (char)Serial1.read();
    Serial2.print(inChar);
  }
  */
}
