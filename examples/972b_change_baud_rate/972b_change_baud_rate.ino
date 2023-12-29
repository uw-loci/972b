#include <972b.h>

PressureTransducer sensor; // Instantiate 972b sensor object with default address "253"

void setup() {
    Serial.begin(9600);   // initialize PC COM interface - pins 0(RX), 1(TX)
    Serial2.begin(9600);  // initialize UART-RS486 transceiver interface - pins 17(RX2), 16(TX2)
   
    String response;

    sensor.sendCommand("BR?"); // Query the default BR
    response = sensor.readResponse();
    Serial.println("Default Baud Rate: " + response);
    
    sensor.changeBaudRate("19200");
    delay(1000); // Delay allow sensor to configure

    sensor.sendCommand("BR?"); // Query the new BR
    response = sensor.readResponse();
    Serial.println("Changed Baud Rate: " + response);
}

void loop() {
}
