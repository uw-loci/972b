#ifndef _972B_H_
#define _972B_H_

#include <Arduino.h>

// Constants
extern const char* const DEFAULT_ADDR;

// Structure for NAK codes
struct NAKCode {
    int code; 
    const char* description;
};

void sendCommand(String deviceAddress=DEFAULT_ADDR, String command="", String parameter="");
String decodeNAK(String codeStr);
String readResponse(String deviceAddress=DEFAULT_ADDR);
void changeBaudRate(String deviceAddress=DEFAULT_ADDR, String newBaudRate="9600");
void setRS485Delay(String deviceAddress=DEFAULT_ADDR, String delaySetting="ON");
void printResponse(const String& response, const String& deviceAddress=DEFAULT_ADDR);
void queryRS485Delay(String deviceAddress=DEFAULT_ADDR);
void printResponse(const String& response, const String& deviceAddress);
void setupSetpoint(String deviceAddress, String setPoint, String direction, String hysteresis, String enableMode);


#endif // _972B_H_