#ifndef _972B_H_
#define _972B_H_

#include <Arduino.h>

// Constants
#define DEFAULT_ADDR "253"
#define DEFAULT_RESPONSE_TIMEOUT 5000
#define DEFAULT_MAX_RESPONSE_CHARACTER_LENGTH 256

// Structure for NAK codes
struct NAKCode {
    int code; 
    const char* description;
};

struct NACKResult {
    String description;
    bool found;
};

class PressureTransducer {
    public:
        PressureTransducer(String addr=DEFAULT_ADDR, Stream& serial=Serial2);

        static int getNumNackCodes();
        void sendCommand(String command="", String parameter="");
        String readResponse();
        void changeBaudRate(String newBaudRate="9600");
        void setRS485Delay(String delaySetting="ON");
        void printResponse(const String& response);
        void queryRS485Delay();
        void setupSetpoint(String setpoint, String direction, String hysteresis, String enableMode);
        String requestPressure(String measureType="PR3");
        void printPressure(String measureType="PR3");
        void setResponseTimeout(unsigned long timeout);

    private:
        Stream& serialPort;
        String deviceAddress;
        static NAKCode nakCodes[];
        NACKResult decodeNAK(String codeStr);
        bool checkForLockError(String response);
        unsigned long responseTimeout = DEFAULT_RESPONSE_TIMEOUT;           // Default timeout (ms)
        const int maxResponseLength = DEFAULT_MAX_RESPONSE_CHARACTER_LENGTH;// Default maximum response character length
};


#endif // _972B_H_