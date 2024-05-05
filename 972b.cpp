#include "972b.h"

const String PressureTransducer::INCOMPLETE_RESPONSE = "ERROR Incomplete response";
const String PressureTransducer::RESPONSE_TOO_LONG = "ERROR Response too long";

PressureTransducer::PressureTransducer(String addr, HardwareSerial& serial)
    : deviceAddress(addr.length() > 0 ? addr : DEFAULT_ADDR), 
      serialPort(serial), responseTimeout(3000) {
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
    if (command.length() == 0) return;
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
                response += c; // add most recent character to response
                if (response.endsWith(";FF")) {
                    break; // correctly formed response received
                }
            } else {
                Serial.println(RESPONSE_TOO_LONG);
                return RESPONSE_TOO_LONG;
            }
        }
    }
    if (response.length() == 0) {
        Serial.println("Response incomplete or timeout.");
        return INCOMPLETE_RESPONSE; // time-out or incomplete response
    }
    return response;
}

String PressureTransducer::parseResponse(const String& response) {
    if (response.length() == 0) {
        return "Error:None";
    }

    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        return response;
    }

    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        int startIndex = response.indexOf("ACK") + 3;
        int endIndex = response.indexOf(';', startIndex);
        if (endIndex != -1) { // found the termination character
            return response.substring(startIndex, endIndex); // return core info
        } else {
            return "Error:ACKmalformed";
        }
    }
    else if (response.startsWith("@" + this->deviceAddress + "NAK")) {
        int startIndex = response.indexOf("NAK") + 3; // Add three to move past "NAK"
        int endIndex = response.indexOf(';', startIndex);
        if (endIndex != -1) { // found the termination character
            return response.substring(startIndex, endIndex);
        } else {
            return "Error:NAKmalformed";
        }
    } else {
        return "UnknownErr"; // unrecognized error
    }
}

CommandResult PressureTransducer::status() {
    CommandResult result; // info variable to return to caller
    result.outcome = false; // Assume failure unless proven otherwise

    sendCommand("T?");
    String response = readResponse();

    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
        return result; // Exit early
    } else if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.outcome = true; // Indicate success to function caller
        result.resultStr = parseResponse(response);
    } else {
        // Indicate failure to function caller
        result.resultStr = parseResponse(response);
    }
    return result;
}

void PressureTransducer::changeBaudRate(String newBaudRate) {
    // Array of valid baud rates
    const long validBaudRates[] = {4800, 9600, 19200, 38400, 57600, 115200, 230400};
    const int numRates = sizeof(validBaudRates) / sizeof(validBaudRates[0]);
    bool isValidRate = false;

    // Check if rate is valid
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

    // Proceed with the command
    sendCommand("BR", newBaudRate);

    // Proceed to change the baud rate if valid
    serialPort.end(); // End current communication
    serialPort.begin(newBaudRate.toInt()); // Start with the new baud rate
    Serial.println("Baud rate changed to: " + newBaudRate);
    String response = readResponse();

    // Check for critical communication errors first
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        Serial.println(response); // Print the error and return immediately
        return;
    }

    String parsedResponse = parseResponse(response);
    if (!parsedResponse.startsWith("Error")) {
        Serial.println("Baud rate change successful: " + parsedResponse);
    } else {
        Serial.println(parsedResponse);  // Print error message handled by parseResponse
    }
}

void PressureTransducer::setRS485Delay(String delaySetting) {
    sendCommand("RSD", delaySetting);
    String response = readResponse();
    String parsedResponse = parseResponse(response);

    if (!parsedResponse.startsWith("Error")) {
        Serial.println("RS485 Delay setting updated to: " + delaySetting);
    } else {
        Serial.println(parsedResponse);
    }
}

void PressureTransducer::queryRS485Delay() {
    sendCommand("RSD?");
    String response = readResponse();

    // Immediately handle communication error responses
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        Serial.println(response);
        return;
    }

    String parsedResponse = parseResponse(response);
    if (!parsedResponse.startsWith("Error")) {
        Serial.println("Current RS485 Delay Setting: " + parsedResponse);
    } else {
        Serial.println(parsedResponse);  // Print error messages handled by parseResponse
    }
}

void PressureTransducer::printResponse(const String& response) { // to serial
    // Immediately handle communication error responses
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        Serial.println(response);
        return;
    }

    String parsedResponse = parseResponse(response);

    if (parsedResponse.startsWith("Error")) {
        Serial.println(parsedResponse); // Print the error message from parseResponse
    } else if (response.startsWith("@" + this->deviceAddress)) {
        if (response.indexOf("ACK") != -1) {
            // If the response is acknowledgment, print it directly
            Serial.println("ACK Response: " + parsedResponse);
        } else if (response.indexOf("NAK") != -1) {
            // The response is negative acknowledgment
            Serial.println("NAK Error: " + parsedResponse);
        }
    } else {
        // Handle unrecognized prefixes or other issues not caught by parseResponse
        Serial.println("No valid response received or unrecognized prefix.");
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
    result.outcome = false; // Assume failure unless proven otherwise

    // Step 1: Set the setpoint value
    sendCommand("SP1", setpoint);
    String response = readResponse();
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
        return result;
    }
    if (!response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.resultStr = parseResponse(response);
        return result; // Early return on failure
    }

    // Step 2: Set the setpoint direction (ABOVE/BELOW)
    sendCommand("SD1", direction);
    response = readResponse();
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.outcome = false;
        result.resultStr = response;
        return result;
    }
    if (!response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.resultStr = parseResponse(response);
        return result; // Early return on failure
    }

    // Step 3: Set the setpoint hysteresis value
    sendCommand("SH1", hysteresis);
    response = readResponse();
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
        return result;
    }
    if (!response.startsWith("@" + this->deviceAddress + "ACK")){
        result.resultStr = parseResponse(response);
        return result; // Early return on failure
    }

    // Step 4: Enable the setpoint
    sendCommand("EN1", enableMode);
    response = readResponse();
    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
        return result; // Early return on failure
    }
    if (!response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.resultStr = parseResponse(response);
        return result; // Early return on failure
    }

    // if everything is ok
    result.outcome = true;
    result.resultStr = "972bOK";
    return result;
}

CommandResult PressureTransducer::requestPressure(String measureType) {
    sendCommand(measureType + "?");
    String response = readResponse();

    CommandResult result;
    result.outcome = false; // Assume failure unless proven otherwise

    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
        return result;
    }

    String parsedResponse = parseResponse(response);
    if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        double pressureValue = sciToDouble(parsedResponse); // Convert to double
        if (isnan(pressureValue)) {
            result.resultStr = "Error:Invalid pressure reading";
        } else {
            result.outcome = true;
            result.resultStr = parsedResponse;
        }
    } else {
        result.resultStr = parsedResponse;
    }
    return result;
}

void PressureTransducer::printPressure(String measureType) {
    sendCommand(measureType + "?");
    String response = readResponse();
    Serial.println(response);
}

CommandResult PressureTransducer::setPressureUnits(String units) {
    sendCommand("U", units);// datasheet p.43
    String response = readResponse();

    CommandResult result; // info to return to caller
    result.outcome = false; // Initially assume the outcome is false

    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
    } else if (response.startsWith("@" + this->deviceAddress + "ACK")){
        result.outcome = true; // Indicate success
        result.resultStr = parseResponse(response);
    } else {
        // Indicate NACK failure to function caller
        result.resultStr = parseResponse(response);
    }
    return result;
}

CommandResult PressureTransducer::setUserTag(String tag) {
    String command = "UT";
    CommandResult result;
    result.outcome = false; // Assume failure unless proven otherwise

    sendCommand(command, tag);
    String response = readResponse();

    if (response == INCOMPLETE_RESPONSE || response == RESPONSE_TOO_LONG) {
        result.resultStr = response;
    } else if (response.startsWith("@" + this->deviceAddress + "ACK")) {
        result.outcome = true;
        result.resultStr = parseResponse(response);
    } else {
        // Indicate failure to caller 
        result.resultStr = parseResponse(response); // for the LCD
    }
    return result;
}

void PressureTransducer::setResponseTimeout(unsigned long timeout) {
        responseTimeout = timeout;
}

// Function to parse string and perform conversion to double 
double PressureTransducer::sciToDouble(String sciString) {
    char buffer[sciString.length() + 1]; // Create a buffer to store the string as a char array
    sciString.toCharArray(buffer, sizeof(buffer)); // Convert string to char array
    return strtod(buffer, NULL); // Convert char array to double handling scientific notation
}
