#include <972b.h>

PressureTransducer sensor; // Instantiate 972b sensor object with default address "253"

void setup() {
    Serial.begin(9600);   // initialize PC COM interface - pins 0(RX), 1(TX)
    Serial2.begin(9600);  // initialize UART-RS486 transceiver interface - pins 17(RX2), 16(TX2)
}

void loop() {
    // Print the MicroPirani sensor reading
    Serial.print("MicroPirani sensor reading:");
    sensor.printPressure("PR1");
    delay(1000);

    // Print the Cold Cathode reading
    Serial.print("Cold Cathode reading:");
    sensor.printPressure("PR2");
    delay(1000);

    // Print combined 3 digit reading
    Serial.print("MicroPirani and Cold Cathode combined 3 digit reading:");
    sensor.printPressure("PR3");
    delay(1000);

    // Print combined 4 digit reading
    Serial.print("MicroPirani and Cold Cathode combined 4 digit reading:");
    sensor.printPressure("PR4");
    delay(1000);
}