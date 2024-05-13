#include <cmath> // For math functions

unsigned long startTime = 0;
bool isDwelling = false;
double pressure = 1013.0; // Start at high pressure

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Mock 972b Debug Sketch Initialized");
  startTime = millis(); // Initialize start time
}

void loop() {
  static String receivedData = "";
  while (Serial2.available() > 0) {
    char inChar = (char)Serial2.read();
    receivedData += inChar;
    if (receivedData.endsWith(";FF")) {
      respondToCommand(receivedData);
      receivedData = "";
    }
  }
}

String formatScientific(double value) {
    char buffer[20];
    sprintf(buffer, "%1.2E", value); // Format as scientific notation

    // Locate 'E' in the formatted string
    char* eLoc = strchr(buffer, 'E'); // Find the exponent 'E'
    if (eLoc != nullptr && *(eLoc + 2) == '0') {
        // If there is a leading zero in the exponent, remove it
        for (char* p = eLoc + 2; *p; p++) {
            *p = *(p + 1); // Shift the characters down to remove the zero
        }
    }

    return String(buffer);
}

void respondToCommand(String command) {
  Serial.print("Received Command: ");
  Serial.println(command);

  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;

  // Check if we are in the dwelling phase or not
  if (!isDwelling) {
    if (elapsedTime < 15000) { // First 15 seconds drop fast to 1.41
      pressure = 1013.0 - (1013.0 - 1.41) * (elapsedTime / 15000.0);
    } else {
      isDwelling = true;
      startTime = currentTime; // Reset start time for dwelling
    }
  } else {
    if (elapsedTime < 5000) { // Dwell for 5 seconds oscillating around 1.41
      pressure = 1.41 + 0.05 * sin(2 * PI * elapsedTime / 5000.0);
    } else if (elapsedTime < 10000) { // Next 5 seconds oscillate around 1013
      pressure = 1013.0 + 5 * sin(2 * PI * (elapsedTime - 5000) / 5000.0);
    } else {
      isDwelling = false; // Reset to drop fast again
      startTime = currentTime;
    }
  }

  if (command == "@253PR3?;FF") {
    String pressureResponse = "@253ACK" + formatScientific(pressure) + ";FF"; 
    Serial2.print(pressureResponse);
    Serial.print("Responded with: ");
    Serial.println(pressureResponse);
  } else {
    // Handle other commands as previously
    String response = "@253NACK180;FF";
    if (command == "@253U!MBAR;FF") {
      response = "@253ACKMBAR;FF";
    } else if (command == "@253UT!EBEAM1;FF") {
      response = "@253ACKEBEAM1;FF";
    } else if (command == "@253T?;FF") {
      response = "@253ACKO;FF";
    } else if (command == "@253SP1!2.00E+0;FF") {
      response = "@253ACK2.00E+0;FF";
    } else if (command == "@253SD1!BELOW;FF") {
      response = "@253ACKBELOW;FF";
    } else if (command == "@253SH1!2.10E+0;FF") {
      response = "@253ACK2.10E+0;FF";
    } else if (command == "@253EN1!ON;FF") {
      response = "@253ACKON;FF";
    }
    Serial2.print(response);
    Serial.print("Responded with: ");
    Serial.println(response);
  }
}
