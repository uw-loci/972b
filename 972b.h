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
    char description[DEFAULT_MAX_RESPONSE_CHARACTER_LENGTH];
    bool found;
};

class PressureTransducer {
    public:
        PressureTransducer(const char* addr=DEFAULT_ADDR, Stream& serial=Serial2);

        static int getNumNackCodes();
        void sendCommand(const char* command="", const char* parameter="");
        char* readResponse();
        void changeBaudRate(const char* newBaudRate="9600");
        void setRS485Delay(const char* delaySetting="ON");
        void printResponse(const char* response);
        void queryRS485Delay();
        void setupSetpoint(const char* setpoint, const char* direction, const char* hysteresis, const char* enableMode);
        char* requestPressure(const char* measureType="PR3");
        void printPressure(const char* measureType="PR3");
        void setResponseTimeout(unsigned long timeout);

    private:
        Stream& serialPort;
        char deviceAddress[4];
        static NAKCode nakCodes[];
        NACKResult decodeNAK(const char* codeStr);
        bool checkForLockError(const char* response);
        unsigned long responseTimeout = DEFAULT_RESPONSE_TIMEOUT;           // Default timeout (ms)
        const int maxResponseLength = DEFAULT_MAX_RESPONSE_CHARACTER_LENGTH;// Default maximum response character length
};


#endif // _972B_H_