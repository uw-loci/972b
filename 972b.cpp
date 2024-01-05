#include "972b.h"

PressureTransducer::PressureTransducer(String addr, Stream& serial)
    : deviceAddress(addr.length() > 0 ? addr : DEFAULT_ADDR), 
      serialPort(serial) {
}

// List of possible NACK codes
NAKCode PressureTransducer::nakCodes[] = {
    {8, "Zero adjustment at too high pressure"},
    {9, "Atmospheric adjustment at too low pressure"},
    {160, "Unrecognized message"},
    {169, "Invalid argument"},
    {172, "Value out of range"},
    {175, "Command/query character invalid"},
    {180, "Protected setting (locked)"},
    {195, "Control setpoint enabled (ENC)"}
};

int PressureTransducer::getNumNackCodes() {
    return sizeof(nakCodes) / sizeof(NAKCode);
}

void PressureTransducer::sendCommand(String command, String parameter) {
    String fullCommand = "@" + this->deviceAddress + command;
    if (command.endsWith("?")) {  // Query
        fullCommand += ";FF";
    } else {  // Command
        fullCommand += "!" + parameter + ";FF";
    }
    serialPort.print(fullCommand);
    Serial.println("Sent command: " + fullCommand);
}

// Function to find the description for a given NAK code
NACKResult PressureTransducer::decodeNAK(String codeStr) {
    int code  = codeStr.toInt(); // Convert the String to an integer
    NACKResult result;
    result.found = false;

    for (unsigned int i = 0; i < sizeof(nakCodes) / sizeof(NAKCode); i++) {
        if (nakCodes[i].code == code) {
            result.description = nakCodes[i].description;
            result.found = true;
        }
    }

    if (!result.found) {
        result.description = "Unknown NAK code";
    }
    return result;
}

String PressureTransducer::readResponse() {
    String response = "";
    long startTime = millis();
    
    while (millis() - startTime < responseTimeout && response.length() < maxResponseLength) {  // what
        if (serialPort.available()) {
            char c = serialPort.read();
            response += c;
            if (response.endsWith(";FF")) {
                break;
            }
        }
    }

    if (response.length() > maxResponseLength && !response.endsWith(";FF")) {
        Serial.println("Error: Response too long or incomplete.");
        return "";
    }
    // TODO: Figure out what to do here with logging
    // Check for ACK or NAK
    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        return response; // Successful response
    } else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        return response; // Error in response
    }

    return "No valid response"; // No valid response received
}

void PressureTransducer::changeBaudRate(String newBaudRate) {
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
    sendCommand("BR", newBaudRate);
    String response = readResponse();

    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        Serial.println("Baud rate change successful: " + response);
    } else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        int codeStart = response.indexOf("NAK") + 3;
        int codeEnd = response.indexOf(';', codeStart);
        String codeStr = response.substring(codeStart, codeEnd);
        
        NACKResult nak = decodeNAK(codeStr);
        if (nak.found) {
            Serial.println("Error changing the baud rate. NAK code: " + codeStr + " - " + nak.description);
        } else {
            Serial.println("Error changing baud rate. Unknown NAK code: " + codeStr);
        }
    } else {
        Serial.println("No valid response received for baud rate change.");
    }
}

void PressureTransducer::setRS485Delay(String delaySetting) {
    sendCommand("RSD", delaySetting);
    String response = readResponse();

    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        Serial.println("RS485 Delay setting updated to: " + delaySetting);
    } else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        Serial.println("Error in setting RS485 delay to: " + delaySetting);
        // Additional error handling can be implemented here
    } else {
        Serial.println("No valid response received for setting RS485 delay.");
    }
}

void PressureTransducer::queryRS485Delay() {
    sendCommand("RSD?");
    String response = readResponse();

    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        // Extracting the actual delay setting from the response
        int settingStartIndex = response.indexOf("ACK") + 3; // Position after "ACK"
        int settingEndIndex = response.indexOf(';', settingStartIndex);
        String delaySetting = response.substring(settingStartIndex, settingEndIndex);
        Serial.println("Current RS485 Delay Setting: " + delaySetting);
    } else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        Serial.println("Error in querying RS485 delay setting.");
        // Additional error handling can be implemented here
    } else {
        Serial.println("No valid response received for RS485 delay query.");
    }
}

void PressureTransducer::printResponse(const String& response) {
    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        int settingStartIndex = response.indexOf("ACK") + 3;  // Position after "ACK"
        int settingEndIndex = response.indexOf(';', settingStartIndex);
        String content = response.substring(settingStartIndex, settingEndIndex);
        Serial.println("Response: " + content);
    } else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        Serial.println("Error in response: " + response);
    } else {
        Serial.println("No valid response received.");
    }
}

bool PressureTransducer::checkForLockError(String response) {
    if (response.startsWith("@" + this->deviceAddress + "NAK180")) {
        Serial.println("Error: Transducer is locked. Unlock required to change parameters.");
        return true; // Indicates a lock error was detected
    }
    return false; // No lock error
}

void PressureTransducer::setupSetpoint(String setpoint, String direction, String hysteresis, String enableMode) {
    String response;

    // Step 1: Set the setpoint value
    sendCommand("SP1", setpoint);
    response = readResponse();
    if (checkForLockError(response)) return;
    printResponse(response);

    // Step 2: Set the setpoint direction (ABOVE/BELOW)
    sendCommand("SD1", direction);
    response = readResponse();
    if (checkForLockError(response)) return;
    printResponse(response);

    // Step 3: Set the setpoint hysteresis value
    sendCommand("SH1", hysteresis);
    response = readResponse();
    if (checkForLockError(response)) return;
    printResponse(response);

    // Step 4: Enable the setpoint
    sendCommand("EN1", enableMode);
    response = readResponse();
    if (checkForLockError(response)) return;
    printResponse(response);
}

String PressureTransducer::requestPressure(String measureType) {
    sendCommand(measureType + "?");
    String response = readResponse();
    return response;
}

void PressureTransducer::printPressure(String measureType) {
    sendCommand(measureType + "?");
    String response = readResponse();
    Serial.println(response);
}

void PressureTransducer::setResponseTimeout(unsigned long timeout) {
        responseTimeout = timeout;
}