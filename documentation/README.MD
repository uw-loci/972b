### Sending Commands
A wrapper function around the Serial.print() can be used to send serial queries or commands to the 972b, assuming it’s connected through a UART-RS485 adapter and a serial interface is instantiated (using pins 16 and 17 as described in this note).

`void PressureTransducer::sendCommand(String command, String parameter);`

This function takes two parameters:
    1. command: A String representing the command to send to the 972b
    2. parameter: This is an optional parameter that defaults to an empty String. This is used only for commands and not for queries.
