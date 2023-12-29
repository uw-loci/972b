#ifndef _972B_H_
#define _972B_H_

#include <Arduino.h>

// Constants
#define DEFAULT_ADDR "253"

// Structure for NAK codes
struct NAKCode {
    int code; 
    const char* description;
};

class PressureTransducer {
    public:
        PressureTransducer(String addr=DEFAULT_ADDR);

        static int getNumNackCodes();
        void sendCommand(String command="", String parameter="");
        String readResponse();
        void changeBaudRate(String newBaudRate="9600");
        void setRS485Delay(String delaySetting="ON");
        void printResponse(const String& response);
        void queryRS485Delay();
        void setupSetpoint(String setPoint, String direction, String hysteresis, String enableMode);
        String requestPressure(String measureType="PR3");
        void printPressure(String measureType="PR3");

    private:
        String deviceAddress;
        static NAKCode nakCodes[];
        String decodeNAK(String codeStr);
        bool checkForLockError(String response);
};


#endif // _972B_H_