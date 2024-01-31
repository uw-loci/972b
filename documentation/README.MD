### Sending Commands
A wrapper function around the Serial.print() can be used to send serial queries or commands to the 972b, assuming it’s connected through a UART-RS485 adapter and a serial interface is connected.

`void PressureTransducer::sendCommand(String command, String parameter);`

This function takes two parameters:
1. `command`: A String representing the command to send to the 972b
2. `parameter`: This is an optional parameter that defaults to an empty String. This is used only for commands and not for queries.

Notes:
 - `sendCommand` auto inserts the attention character “@” and ends with the termination characters “;FF” (datasheet, pg 13), requiring only the unique command to be passed.
 - The command string is limited to ASCII characters but is case-insensitive

Example usage [link](https://github.com/mslaffin/972b/blob/main/examples/querying/972b_model_number_query/972b_model_number_query.ino):

TODO:
 - Add generic checking to confirm valid command string is passed

###Receiving a Response
If the sensor successfully acknowledges the command and responds through the transeiver, the native `Serial.read()` function can be used to read input serial data on `Serial2`. The function waits for a response for a specified timeout duration and processes the response to determine its validity and type.

`String PressureTransducer::readResponse();`

This function takes no parameters and returns: 
1. `String`: The response received by the pressure transducer. This could be a successful response (starting with ‘ACK’), an error response (starting with ‘NAK’), or a string indicating no valid response was received.

Notes: 
 - The timeout duration is arbitrary, and may need to be adjusted 
