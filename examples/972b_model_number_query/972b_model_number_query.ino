#include <972b.h>

void setup() {
    Serial.begin(9600);   // initialize PC COM interface - pins 0(RX), 1(TX)
    Serial2.begin(9600);  // initialize UART-RS486 transceiver interface - pins 17(RX2), 16(TX2)
   
   
    String response;

    sendCommand("BR?"); // Query the default BR
    response = readResponse();
    Serial.println("Default Baud Rate: " + response);
    
	changeBaudRate("19200");
    delay(1000); // Delay allow sensor to configure

    sendCommand("BR?"); // Query the new BR
    response = readResponse();
    Serial.println("Changed Baud Rate: " + response);
}

void loop() {
}