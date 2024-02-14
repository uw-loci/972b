# 972b driver documentation



# Basic Communication Flow
This section outlines the fundamental steps for establishing communication with a pressure transducer using the `PressureTransducer` class defined in this library. 
<details>
<summary>Read more about Communication Flow</summary>

### Constructing a sensor object
The `PressureTransducer` class is initialized with a constructor that sets up the device for communication.

`PressureTransducer::PressureTransducer(String addr, Stream& serial);`

Parameters:
 - `addr`: A String representing the device address. If an empty string is provided, a default address is used.
 - `serial`: A reference to a Stream object which represents the serial port used for communication with the transducer. 
The constructor initializes the device with the given address (or defaults to `253` one if not specified) and sets up the specified serial port for communication.

#### Usage
```c
#include <972b.h>

PressureTransducer sensor; // Instantiate 972b sensor object with default address "253"

void setup() {
    Serial.begin(9600);   // initialize PC COM interface
    Serial2.begin(9600);  // initialize UART-RS486 transceiver interface

    String response;
    
    sensor.sendCommand("MD?"); // Query device model number
    response = sensor.readResponse();
    Serial.println("Model Number: " + response);
}

void loop() {
}
```
### Sending Commands
A wrapper function around `Serial.print()` can be used to send serial queries or commands to the 972b, assuming it’s connected through a UART-RS485 adapter and sent to the correct serial interface.

`void PressureTransducer::sendCommand(String command, String parameter);`

This function takes two parameters:
1. `command`: A String representing the command to send to the 972b
2. `parameter`: This is an optional parameter that defaults to an empty String. This is used only for commands and not for queries.

Notes:
 - The default serial port is `Serial2` (pins 18 & 19 on the Arduino Mega). This can be configured to any output stream.
 - `sendCommand` auto inserts the attention character “@” and ends with the termination characters “;FF” (datasheet, pg 13), requiring only the unique command to be passed.
 - The command string is limited to ASCII characters but is case-insensitive

Example usage [link](https://github.com/mslaffin/972b/blob/main/examples/querying/972b_model_number_query/972b_model_number_query.ino):


### Receiving a Response
If the sensor successfully acknowledges the command and responds through the transeiver, `Serial.read()` can be used to read input serial data on `Serial2`. The function waits for a response for a specified timeout duration and processes the response to determine its validity and type.

`String PressureTransducer::readResponse();`

This function takes no parameters and returns: 
1. `String`: The response received by the pressure transducer. This could be a successful response (starting with ‘ACK’), an error response (starting with ‘NAK’), or a string indicating no valid response was received.

Notes: 
 - The timeout duration is arbitrary, and may need to be adjusted 
</details>



# Communication Setup
This section provides an overview of configuring and querying the 972b transducer's communication settings, including baud rate, device address, and communication delay.
<details>
<summary>Read more about Communication Setup</summary>

This section details the commands for configuring and querying the communication settings of the 972b transducer. These commands allow for adjusting parameters such as baud rate, device address, and communication delay settings.


## Querying Communication Baud Rate
To query the current baud rate setting, use:
```c
sensor.sendCommand("BR?"); // Query current baud rate
```
- Expected response: `@xxxACK9600;FF`
- Explanation: Returns the current communication baud rate.
- The response codes (ACK, NAK) indicate successful or unsuccessful execution of the command, respectively.

## Setting Communication Baud Rate
To set the communication baud rate, use the following command:
```c
sensor.changeBaudRate("9600"); // Set baud rate to 9600
```
- Explanation: Sets the communication baud rate. Valid options include 4800, 9600, 19200, 38400, 57600, 115200, and 230400.

## Setting Transducer Address
To set the transducer communication address, use this command:
```c
sensor.sendCommand("AD!123"); // Set address to 123
```
- Expected response: `@xxxACK123;FF`
- Explanation: Sets the transducer's communication address. Valid addresses range from 001 to 253.

## Setting Communication Delay
To toggle the communication delay between receiving and transmitting, use:
```c
sensor.sendCommand("RSD!OFF"); // Turn off communication delay
```
- Expected response: `@xxxACKOFF;FF`
- Explanation: Turns on or off the communication delay between receiving and sending data.
</details>

# Transducer Information
This section provides examples of querying the status, model, and version information of the sensor system. Details are provided on viewing the device's operational lifetime programming a custom user tag.
<details>
<summary>Read more about Transducer Information</summary>

## Querying the Model Number (972B)
To query the system model number, use:
```c
sensor.sendCommand("MD?"); // Query model number
```
- Expected response: `@xxxACK972B;FF`
- Explanation: Responds with the device model number.

## Querying the Device type name (MicroPirani)
To query the device type name, use:
```c
sensor.sendCommand("DT?"); // Query device type
```
- Expected response: `@xxxACKDualMag;FF`
- Explanation: Responds with the device sensor type.

## Querying the Manufacturer Name
To query the manufacturer name, use:
```c
sensor.sendCommand("MF?"); // Query manufacturer name
```
- Expected response: `@xxxACKMKS;FF`
- Explanation: Responds with the Manufacturer name (MKS Instruments).

## Querying the Hardware version
To query the hardware version, use:
```c
sensor.sendCommand("HV?"); // Query hardware version
```
- Example response: `@xxxACKA;FF`
- Explanation: Responds with the hardware version.

## Querying the Firmware version
To query the firmware version, use:
```c
sensor.sendCommand("FV?"); // Query firmware version
```
- Example response: `@xxxACK1.12;FF`
- Explanation: Responds with the firmware version.

## Querying the Serial number
To query the serial number, use:
```c
sensor.sendCommand("SN?"); // Query serial number
```
- Example response: `@xxxACK08350123456;FF`
- Explanation: Responds with the serial number.

## Querying device switch enable status
To query if device switch is enabled, use:
```c
sensor.sendCommand("SW?"); // Query enable status of device Switch
```
- Expected response: `@xxxACKON;FF`
- Explanation: Responds with the switch enable status. (ON - Enabled)

## Querying hours of operation
To query the number of hours the device has been on, use:
```c
sensor.sendCommand("TIM?"); // Query time on
```
- Example response: `@xxxACK137;FF`
- Explanation: Responds with the number of hours the device has been in operation.

## Querying hours of operation (cold cathode high voltage)
To query the number of hours the CC sensor been on, use:
```c
sensor.sendCommand("TIM2?"); // Query cold cathode time on
```
- Example response: `@xxxACK12;FF`
- Explanation: Responds with the number of hours the cold cathode sensor has been in operation.

## Querying the Cold Cathode "pressure dose"
To query the Cold Cathode time-on as a function of the pressure, use:
```c
sensor.sendCommand("TIM3?"); // Query cold cathode pressure dose
```
- Example response: `@xxxACK1.00E-2;FF`
- Explanation: Responds with an indicator that can be used to determine when maintenance and service are required. If the cold cathode is on for 100 hours at 1E-4 Torr the pressure dose is 100×1E-4 = 1E-2 Torr hours. 

## Querying the MicroPirani sensor temperature
To query the MicroPirani sensor temperature, use:
```c
sensor.sendCommand("TEM?"); // Query MicroPirani temperature
```
- Example response: `@xxxACK2.50E+1;FF`
- Explanation: Responds with the temperature of the MicroPirani sensor.

## Querying the User-Programmed Tag
To query the user-programmed sensor tag, use:
```c
sensor.sendCommand("UT?"); // Query user tag
```
- Example response: `@xxxACKLINECTRA1;FF`
- Explanation: Responds with the user programmed text string.

## Set the User Tag
To set the user-defined sensor tag, use:
```c
sensor.sendCommand("UT!LINECTRA1"); // Set user tag
```
- Example response: `@xxxACKLINECTRA1;FF`
- Explanation: Responds with the user programmed text string.

## Perform a sensor status check
To perform a basic status check, use:
```c
sensor.sendCommand("T?"); // Status check
```
- Expected response: `@xxxACKO;FF`
- Explanation: Responds with a basic check for communication purposes.

</details>

<details>
<summary>Read more about Pressure Reading</summary>
</details>

<details>
<summary>Read more about Setpoint Information</summary>
</details>

<details>
<summary>Read more about Calibration and Adjustment</summary>
</details>

<details>
<summary>Read more about Cold Cathode setup</summary>
</details>

<details>
<summary>Read more about User Switch</summary>
</details>

<details>
<summary>Read more about Information Setup</summary>
</details>
