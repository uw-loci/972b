#include "972b.h"

PressureTransducer sensor; // Assuming default address and Serial2

void setup() {
  Serial.begin(9600);    // Start the Serial communication
  while (!Serial) {
    ; // wait for com port to connect. Needed for native USB
  }
  
  delay(1000); // Wait a sec for everything to stabilize
  querySensor("DT", "Device Type");
  querySensor("FV", "Firmware Version");
  querySensor("HV", "Hardware Version");
  querySensor("MF", "Manufacturer");
  querySensor("MD", "Model");
  querySensor("PN", "Part Number");
  querySensor("SN", "Serial Number");
  querySensor("TIM", "Time ON");                                 // The TIM command returns the number of hours the transducer has been on
  querySensor("TIM2", "Time ON 2 (Cold Cathode sensor)");        // The TIM2 command returns the number of hours the cold cathode sensor has been on
  querySensor("TIM3", "Time ON 3 (Cold Cathode pressure dose)"); // The TIM3 command returns the cold cathode pressure dose
  querySensor("TEM", "Temperature");                             // The TEM command returns the MicroPirani on chip sensor temperature (typical within ±3C)
  querySensor("T", "Transducer Status");                         // Returns the sensor status as O for OK, M for MicroPirani failure, C for Cold Cathode failure, R pressure dose setpoint exceeded or G for Cold Cathode ON.
}

void loop() {
  // Since this is a one-time query, there's no code in loop()
}

void querySensor(String command, String description) {
  sensor.sendCommand(command + "?");
  String response = sensor.readResponse();
  Serial.println(description + ": " + response);
}