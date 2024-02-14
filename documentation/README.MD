# 972b driver documentation

## Basic Communication
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



## Communication setup
This section provides an overview of configuring and querying the 972b transducer's communication settings, including baud rate, device address, and communication delay.
<details>
<summary>Communication setup</summary>

This section details the commands for configuring and querying the communication settings of the 972b transducer. These commands allow for adjusting parameters such as baud rate, device address, and communication delay settings.


# Querying Communication Baud Rate
To query the current baud rate setting, use:
```c
sensor.sendCommand("BR?"); // Query current baud rate
```
- Response: @xxxACK9600;FF
- Explanation: Returns the current communication baud rate.
- The response codes (ACK, NAK) indicate successful or unsuccessful execution of the command, respectively.

# Setting Communication Baud Rate
To set the communication baud rate, use the following command:
```c
sensor.changeBaudRate("9600"); // Set baud rate to 9600
```
- Explanation: Sets the communication baud rate. Valid options include 4800, 9600, 19200, 38400, 57600, 115200, and 230400.

# Setting Transducer Address
To set the transducer communication address, use this command:
```c
sensor.sendCommand("AD!123"); // Set address to 123
```
- Response: @xxxACK123;FF
- Explanation: Sets the transducer's communication address. Valid addresses range from 001 to 253.

# Setting Communication Delay
To toggle the communication delay between receiving and transmitting, use:
```c
sensor.sendCommand("RSD!OFF"); // Turn off communication delay
```
- Response: @xxxACKOFF;FF
- Explanation: Turns on or off the communication delay between receiving and sending data.
</details>

<details>
<summary>Pressure Reading</summary>
</details>

<details>
<summary>Setpoint Information</summary>
</details>

<details>
<summary>Calibration and Adjustment</summary>
</details>

<details>
<summary>Cold Cathode setup</summary>
</details>

<details>
<summary>User Switch</summary>
</details>

<details>
<summary>Information Setup</summary>
</details>
