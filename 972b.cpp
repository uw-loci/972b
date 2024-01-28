#include "972b.h"

PressureTransducer::PressureTransducer(const char* addr, Stream& serial)
    : serialPort(serial) {
    strncpy(deviceAddress, addr, sizeof(deviceAddress));
    deviceAddress[sizeof(deviceAddress) - 1] = '\0'; // Ensure null-terminated string
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

void PressureTransducer::sendCommand(const char* command, const char* parameter) {
    char fullCommand[maxResponseLength];
    fullCommand[0] = '\0';
    strcat(fullCommand, "@");
    strcat(fullCommand, deviceAddress);
    strcat(fullCommand, command);

    if(strstr(command, "?")){
        strcat(fullCommand, ";FF");
    } else { // TODO: add handling if command character is explicitly included 
        strcat(fullCommand, "!");
        strcat(fullCommand, parameter);
        strcat(fullCommand, ";FF");
    }

    serialPort.print(fullCommand); // Send the command over UART

    Serial.print("Sent command: ");
    Serial.println(fullCommand);
}

// Function to find the description for a given NAK code
NACKResult PressureTransducer::decodeNAK(const char* codeStr) {
    //int code  = codeStr.toInt(); // Convert the String to an integer
    int code = atoi(codeStr); // Convert the char array to an integer
    NACKResult result;
    result.found = false;

    for (unsigned int i = 0; i < sizeof(nakCodes) / sizeof(NAKCode); i++) {
        if (nakCodes[i].code == code) {
            result.description = nakCodes[i].description;
            result.found = true;
        }
    }

    if (!result.found) {
        strncpy(result.description, "Unknown NACK code", sizeof(result.description)); // Default description
        result.description[sizeof(result.description) - 1] = '\0'; // Ensure null-terminated string
    }
    return result;
}

char* PressureTransducer::readResponse() {
    char response[maxResponseLength];
    response[0] = '\0' // Initialize the response as an empty string
    long startTime = millis();
    
    while (millis() - startTime < responseTimeout && strlen(response) < maxResponseLength) {  // what
        if (serialPort.available()) {
            char c = serialPort.read();
            strncat(response, &c, 1); // Append character to the reponse
            if(strstr(response, ";FF")) {
                break;
            }
        }
    }

    if (strlen(response) > maxResponseLength && !strstr(response, ";FF")) {
        Serial.println("Error: Response too long or incomplete.");
        return "";
    }
    // TODO: Figure out what to do here with logging
    // Check for ACK or NAK
    if (strstr(response "@" + this->deviceAddress + "ACK")) {
        return response; // Successful response
    } else if (strstr(response, "@" + this->deviceAddress + "NAK")) {
        return response; // Error in response
    }

    return "No valid response"; // No valid response received
}

void PressureTransducer::changeBaudRate(const char* newBaudRate) {
    // Array of valid baud rates
    const long validBaudRates[] = {4800, 9600, 19200, 38400, 57600, 115200, 230400};
    const int numRates = sizeof(validBaudRates) / sizeof(validBaudRates[0]);

    // Check if the new BaudRate is valid
    bool isValidRate = false;
    for (int i = 0; i < numRates; i++) {
        if (atoi(newBaudRate) == validBaudRates[i]) {
            isValidRate = true;
            break;
        }
    }

    if (!isValidRate) {
        Serial.println("Invalid baud rate: " + String(newBaudRate));
        return;
    }

    // If the rate is valid, proceed with the command
    sendCommand("BR", newBaudRate);
    char* response = readResponse();

    if (response == NULL) {
        Serial.println("Error: No valid response received for baud rate change.");
        return; // Exit early if there's a problem reading the response
    }

    if (strstr(response, "@" + this->deviceAddress + "ACK")) {
        Serial.println("Baud rate change successful: " + String(response));
    } else if (strstr(response, "@" + this->deviceAddress + "NAK")) {
        int codeStart = response - strstr(response, "NAK") + 3;
        int codeEnd = strstr(response, ';') - response;
        char codeStr[4]; // Char array to store codeStr
        strncpy(codeStr, response + codeStart, codeEnd);
        codeStr[codeEnd] = '\0'; // Ensure null-terminated string
        
        NACKResult nak = decodeNAK(codeStr);
        if (nak.found) {
            Serial.println("Error changing the baud rate. NAK code: " + String(codeStr) + " - " + String(nak.description));
        } else {
            Serial.println("Error changing baud rate. Unknown NAK code: " + String(codeStr));
        }
    } else {
        Serial.println("No valid response received for baud rate change.");
    }

    // Free up the memory allocated for the response
    free(response);
}

void PressureTransducer::setRS485Delay(const char* delaySetting) {
    sendCommand("RSD", delaySetting);
    char* response = readResponse();

    if (strstr(response, "@" + this->deviceAddress + "ACK")) {
        Serial.print("RS485 Delay setting updated to: ");
        Serial.println(delaySetting);
    } else if (strstr(response, "@" + this->deviceAddress + "NAK")) {
        Serial.print("Error in setting RS485 delay to: ");
        Serial.println(delaySetting);
    } else {
        Serial.println("No valid response received for setting RS485 delay.");
    }

    free(response);
}

void PressureTransducer::queryRS485Delay() {
    sendCommand("RSD?");
    const* response = readResponse();

    if (strstr(response, "@" + this->deviceAddress + "ACK")) {
        // Extracting the actual delay setting from the response
        char* ackPtr = strstr(response, "ACK");
        if (ackPtr != NULL) {
            char* settingStartPtr = ackPtr + 3;
            char* semicolonPtr = strchr(settingStartPtr, ';');
            if (semicolonPtr != NULL){
                *semicolonPtr = '\0'; // Null terminate to extract the setting
                Serial.print("Current RS485 delay setting: ");
                Serial.println(settingStartPtr);
            }
        }
    } else if (strstr(response, "@" + this->deviceAddress + "NAK")) {
        Serial.println("Error in querying RS485 delay setting.");
        // Additional error handling can be implemented here
    } else {
        Serial.println("No valid response received for RS485 delay query.");
    }

    free(response);
}

void PressureTransducer::printResponse(const char* response) {
    // Check basic format of response
    const char* ackPattern = "@";
    strcat(ackPattern, this->deviceAddress);
    strcat(ackPattern, "ACK");

    const char* nakPattern = "@";
    strcat(nakPattern, this->deviceAddress);
    strcat(nakPattern, "NAK");

    if (strncmp(response, ackPattern, strlen(ackPattern)) == 0) {
        // find position of "ACK" in the response
        const char* ackPtr = strstr(response, "ACK");
        if (ackPtr != NULL) {
            const char* settingStartPtr = ackPtr + 3; // Position after "ACK"
            const char* semicolonPtr = strchr(settingStartPtr, ';');
            if (semicolonPtr != NULL) {
                size_t contentLength = semicolonPtr - settingStartPtr;
                char content[contentLength + 1]; // +1 for null terminator
                strncpy(content, settingStartPtr, contentLength);
                content[contentLength] = '\0';
                Serial.print("Response: ");
                Serial.print(content);
            }
        }
    } else if (strncmp(response, nakPattern, strlen(nakPattern)) == 0){
        Serial.print("Error in response: ");
        Serial.println(response);
    } else {
        Serial.println("No valid response received.");
    }
}

bool PressureTransducer::checkForLockError(String response) {
    const int bufferSize = 20; // "@253NAK180" + null terminator + margin 
    char lockErrorPattern[bufferSize];

    // Construct the lock error pattern
    strcpy(lockErrorPattern, "@");
    strcat(lockErrorPattern, this->deviceAddress);
    strcat(lockErrorPattern, "NAK180");

    // check if the response starts with the lock error pattern 
    if (strncmp(response, lockErrorPattern, strlen(lockErrorPattern)) == 0) {
        Serial.println("Error: Transducer is locked. Unlock required to change parameters.");
        return true; // Indicates a lock error was detected
    }
    return false; // No lock error
}

void PressureTransducer::setupSetpoint(String setpoint, String direction, String hysteresis, String enableMode) {
    char* response;

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