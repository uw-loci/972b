#include <972b.h>

PressureTransducer sensor; // Instantiate 972b sensor object with default address "253"

void setup() {
    Serial.begin(9600);   // initialize PC COM interface
    Serial2.begin(9600);  // initialize UART-RS486 transceiver interface

    PressureTransducer sensor("253", Serial2); // Set a 5-second timeout for example

    sensor.setResponseTimeout(3000);
    
    String response;
    sensor.sendCommand("MD?"); // Query device model number
    response = sensor.readResponse();
    Serial.println("Model Number: " + (response.isEmpty() ? "No response" : response));
}

void loop() {
}