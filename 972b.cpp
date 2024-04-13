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
    command.trim(); // Remove any leading or trailing whitespace
    
    String fullCommand = "@" + this->deviceAddress + command;
    if (command.endsWith("?")) {  // it's a Query
        fullCommand += ";FF";
    } else {  // it's a Command
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
    
    while (millis() - startTime < responseTimeout) {
        if (serialPort.available()) {
            char c = serialPort.read();
            if (response.length() < maxResponseLength) {
                response += c;
                if (response.endsWith(";FF")) {
                    break;
                }
            } else {
                Serial.println("Error: Response too long.");
                return "";
            }
        }
    }

    if (!response.endsWith(";FF")) {
        Serial.println("Error: Incomplete response.");
        return "";
    }
    // TODO: Figure out what to do here with logging
    // Check for ACK or NAK
    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        return response; // Successful response
    } else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        return response; // Error in response
    }

    Serial.println("Error: No valid response received.");
    return ""; // No valid response received
}

CommandResult PressureTransducer::status() {
    CommandResult result; // info to return to caller

    sendCommand("T?");
    String response = readResponse();

    if (response.startsWith("@" + this->deviceAddress + "ACK")){
        result.outcome = true
        result.resultStr = parseResponse(response); // to print on the LCD
    } else {
        result.outcome = false; // Indicate failure to function caller
        result.resultStr = parseResponse(response); // to print on the LCD
    }
    return result;
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

CommandResult PressureTransducer::setupSetpoint(String setpoint, String direction, String hysteresis, String enableMode) {
    CommandResult result;

    // Step 1: Set the setpoint value
    sendCommand("SP1", setpoint);
    String response = readResponse();
    if (!response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.outcome = false;
        result.resultStr = parseError(response);
        return result;
    }

    // Step 2: Set the setpoint direction (ABOVE/BELOW)
    sendCommand("SD1", direction);
    response = readResponse();
    if (!response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.outcome = false;
        result.resultStr = parseError(response);
        return result;
    }

    // Step 3: Set the setpoint hysteresis value
    sendCommand("SH1", hysteresis);
    response = readResponse();
    if (!response.startsWith("@" + this->deviceAddress + "ACK")){
        result.outcome = false;
        result.resultStr = parseError(response);
        return result;
    }

    // Step 4: Enable the setpoint
    sendCommand("EN1", enableMode);
    response = readResponse();
    if (!response.startsWith("@" + this->deviceAddress + "ACK")){
        result.outcome = false;
        result.resultStr = parseError(response);
        return result;
    }
}

// TODO: think about this function
double PressureTransducer::requestPressure(String measureType) {
    sendCommand(measureType + "?");
    String response = readResponse();

    // Error checking
    if (response.indexOf("NAK") != -1){
        return -1.0; // Error detected
    }

    // Extracting the pressure value from the response
    int startIdx = response.indexOf("ACK") + 3;
    int endIdx = response.indexOf(';', startIdx);
    if (startIdx > 2 && endIdx > startIdx) {
        String pressureStr = response.substring(startIdx, endIdx);
        double pressure = pressureStr.toDouble(); // this method can convert scientific notation to a double representation
        return pressure;
    } else {
        return -1.0; // TODO: develop labview interface
    }
    return response;
}

void PressureTransducer::printPressure(String measureType) {
    sendCommand(measureType + "?");
    String response = readResponse();
    Serial.println(response);
}

String PressureTransducer::parseResponse(const String& response) {
    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        int startIndex = response.indexOf("ACK") + 3; // Add three to move past "ACK"
        int endIndex = response.indexOf(';', startIndex);
        return response.substring(startIndex, endIndex);
    }
    if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        int startIndex = response.indexOf("NAK") + 3; // Add three to move past "NAK"
        int endIndex = response.indexOf(';', startIndex);
        return response.substring(startIndex, endIndex);
    } else {
        return "UnknownErr"; // unrecognized error
    }
}

CommandResult PressureTransducer::setPressureUnits(String units) {

    String command = "U"; // datasheet p.43
    CommandResult result; // info to return to caller

    sendCommand(command, units);
    String response = readResponse();

    if (response.startsWith("@" + this->deviceAddress + "ACK")){
        result.outcome = true; // Indicate success
        result.resultStr = parseResponse(response);
    } else {
        result.outcome = false; // Indicate failure to function caller

        if (response.startsWith("@" + this->deviceAddress + "NAK")) {
            // Extract output from response
            int startIndex = response.indexOf("NAK");
            int endIndex = response.indexOf(';', startIndex);
            result.resultStr = response.substring(startIndex, endIndex);
        } else {
            response.resultStr = "UnknwnErr"; // unrecognized error
        }
    }
    return result;
}

CommandResult PressureTransducer::setUserTag(String tag) {
    String command = "UT";
    sendCommand(command, tag);
    String response = readResponse();

    CommandResult result;

    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.outcome = SUCCESS;
    } else {
        result.outcome = false; // Indicate failure to caller

        if (response.startsWith("@" + this->deviceAddress + "NAK")) {
            // Extract output from response
            int startIndex = response.indexOf("NAK");
            int endIndex = response.indexOf(';', startIndex);
            result.resultStr = response.substring(startIndex, endIndex);
        } else {
            response.resultStr = "UnknownErr"; // unrecognized error
        }
    }
    return result;
}

void PressureTransducer::setResponseTimeout(unsigned long timeout) {
        responseTimeout = timeout;
}

