#include <HardwareSerial.h>

const int baudRate = 9600;
const String deviceAddress = "253"; // Transducer's default address

void setup() {
    Serial.begin(baudRate);
    Serial2.begin(baudRate, SERIAL_8N1, 16, 17); // Assuming RX2 is on pin 16, TX2 is on pin 17
}

void loop() {
    if (Serial2.available()) {
        String command = Serial2.readStringUntil(';');
        command.trim(); // Trim any whitespace or newlines
        
        // Mock response for correct model number query
        if (command == "@" + deviceAddress + "MD?") {
            String response = "@" + deviceAddress + "ACK972B;FF";
            Serial2.print(response);
            Serial.println("Sent: " + response); // Echo to debug serial
        }
        // Edge case: command is incorrect or not recognized
        else {
            String nakResponse = "@" + deviceAddress + "NAK160;FF"; // NAK code 160 for unrecognized message
            Serial2.print(nakResponse);
            Serial.println("Sent: " + nakResponse); // Echo to debug serial
        }
    }
}
