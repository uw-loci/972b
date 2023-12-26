#include <Arduino.h>

const char* const DEFAULT_ADDR = "253";

// Define a structure to hold NAK codes and their descriptions
struct NAKCode {
    int code;
    const char* description;
};

// Initialize an array with the NAK codes and their descriptions
NAKCode nakCodes[] = {
    {8, "Zero adjustment at too high pressure"},
    {9, "Atmospheric adjustment at too low pressure"},
    {160, "Unrecognized message"},
    {169, "Invalid argument"},
    {172, "Value out of range"},
    {175, "Command/query character invalid"},
    {180, "Protected setting (locked)"},
    {195, "Control setpoint enabled (ENC)"}
};

void sendCommand(String deviceAddress=DEFAULT_ADDR, String command, String parameter="");
String readResponse(String deviceAddress=DEFAULT_ADDR);
void changeBaudRate(String deviceAddress=DEFAULT_ADDR, String newBaudRate);
String decodeNAK(String code);
void setRS485Delay(String deviceAddress=DEFAULT_ADDR, String delaySetting);
void printResponse(const String& response, const String& deviceAddress=DEFAULT_ADDR);


void setup() {
    Serial.begin(9600);   // initialize PC COM interface
    Serial2.begin(9600);  // initialize UART-RS486 transceiver interface
   
    sendCommand("MD?"); // Query device model number
}

void loop() {
}

void sendCommand(String deviceAddress=DEFAULT_ADDR, String command, String parameter = "") {
    String fullCommand = "@" + deviceAddress + command;
    if (command.endsWith("?")) {  // Query
        fullCommand += ";FF";
    } else {  // Command
        fullCommand += "!" + parameter + ";FF";
    }
    Serial2.print(fullCommand);
    Serial.println("Sent command: " + fullCommand);
}

// Function to find the description for a given NAK code
String decodeNAK(String codeStr) {
    int code  = codeStr.toInt(); // Convert the String to an integer
    
    for (unsigned int i = 0; i < sizeof(nakCodes) / sizeof(NAKCode); i++) {
        if (nakCodes[i].code == code) {
            return nakCodes[i].description;
        }
    }
    return "Unknown NAK code";  // Return this if the code is not found
}

String readResponse(String deviceAddress) {
    String response = "";
    long startTime = millis();
    while (millis() - startTime < 5000) {  // 5-second timeout for response
        if (Serial2.available()) {
            char c = Serial2.read();
            response += c;
            if (response.endsWith(";FF")) {
                break;
            }
        }
    }

    // Check for ACK or NAK
    if (response.startsWith("@" + deviceAddress + "ACK")) {
        return response; // Successful response
    } else if (response.startsWith("@" + deviceAddress + "NAK")) {
        return response; // Error in response
    }

    return "No valid response"; // No valid response received
}

void changeBaudRate(String deviceAddress, String newBaudRate) {
    // Array of valid baud rates
    const long validBaudRates[] = {4800, 9600, 19200, 38400, 57600, 115200, 230400};
    const int numRates = sizeof(validBaudRates) / sizeof(validBaudRates[0]);

    // Check if the new BaudRate is valid
    bool isValidRate = false;
    for (int i = 0; i < numRates; i++) {
        if (newBaudRate.toInt() == validBaudRates[i]) {
            isValidRate = true;
            break;
        }
    }

    if (!isValidRate) {
        Serial.println("Invalid baud rate: " + newBaudRate);
        return;
    }

    // If the rate is valid, proceed with the command
    sendCommand(deviceAddress, "BR", newBaudRate);
    String response = readResponse(deviceAddress);

    if (response.startsWith("@" + deviceAddress + "ACK")) {
        Serial.println("Baud rate change successful: " + response);
    } else if (response.startsWith("@" + deviceAddress + "NAK")) {
        int codeStart = response.indexOf("NAK") + 3;
        int codeEnd = response.indexOf(';', codeStart);
        String codeStr = response.substring(codeStart, codeEnd);
        String codeDesc = decodeNAK(codeStr);

        Serial.println("Error changing baud rate. NAK code: " + codeStr + " - " + codeDesc);
    } else {
        Serial.println("No valid response received for baud rate change.");
    }
}

void setRS485Delay(String deviceAddress, String delaySetting) {
    sendCommand(deviceAddress, "RSD", delaySetting);
    String response = readResponse(deviceAddress);

    if (response.startsWith("@" + deviceAddress + "ACK")) {
        Serial.println("RS485 Delay setting updated to: " + delaySetting);
    } else if (response.startsWith("@" + deviceAddress + "NAK")) {
        Serial.println("Error in setting RS485 delay to: " + delaySetting);
        // Additional error handling can be implemented here
    } else {
        Serial.println("No valid response received for setting RS485 delay.");
    }
}

void queryRS485Delay(String deviceAddress) {
    sendCommand(deviceAddress, "RSD?");
    String response = readResponse(deviceAddress);

    if (response.startsWith("@" + deviceAddress + "ACK")) {
        // Extracting the actual delay setting from the response
        int settingStartIndex = response.indexOf("ACK") + 3; // Position after "ACK"
        int settingEndIndex = response.indexOf(';', settingStartIndex);
        String delaySetting = response.substring(settingStartIndex, settingEndIndex);
        Serial.println("Current RS485 Delay Setting: " + delaySetting);
    } else if (response.startsWith("@" + deviceAddress + "NAK")) {
        Serial.println("Error in querying RS485 delay setting.");
        // Additional error handling can be implemented here
    } else {
        Serial.println("No valid response received for RS485 delay query.");
    }
}

void printResponse(const String& response, const String& deviceAddress) {
    if (response.startsWith("@" + deviceAddress + "ACK")) {
        int settingStartIndex = response.indexOf("ACK") + 3;  // Position after "ACK"
        int settingEndIndex = response.indexOf(';', settingStartIndex);
        String content = response.substring(settingStartIndex, settingEndIndex);
        Serial.println("Response: " + content);
    } else if (response.startsWith("@" + deviceAddress + "NAK")) {
        Serial.println("Error in response: " + response);
    } else {
        Serial.println("No valid response received.");
    }
}

void setupSetpoint(String deviceAddress, String setPoint, String direction, String hysteresis, String enableMode) {
    // Step 1: Set the setpoint value
    sendCommand(deviceAddress, "SP1", setPoint);
    String response = readResponse(deviceAddress);
    printResponse(response, deviceAddress);

    // Step 2: Set the setpoint direction (ABOVE/BELOW)
    sendCommand(deviceAddress, "SD1", direction);

    response = readResponse(deviceAddress);
    printResponse(response, deviceAddress);
    // Step 3: Set the setpoint hysteresis value
    sendCommand(deviceAddress, "SH1", hysteresis);
    response = readResponse(deviceAddress);
    printResponse(response, deviceAddress);

    // Step 4: Enable the setpoint
    sendCommand(deviceAddress, "EN1", enableMode);
    response = readResponse(deviceAddress);
    printResponse(response, deviceAddress);
}
