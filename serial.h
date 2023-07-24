#ifndef serial_h
#define serial_h

#include <Arduino.h>


#define SERIAL_MESSAGE_START 0b11111111

bool sendSerialRequest(const String& msg);

String receiveSerialMessage();

void waitForSerial();

void splitIntToBytes(const int number, char* bytes);

int joinBytesToInt(const char* bytes);


#endif