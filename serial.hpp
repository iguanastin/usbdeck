#ifndef serial_h
#define serial_h

#include <Arduino.h>


#define SERIAL_MESSAGE_START 0b11111111
#define SERIAL_REQUEST_IDENTIFY 0
#define SERIAL_RESPOND_IDENTIFY 1
#define SERIAL_REQUEST_CONFIG 2
#define SERIAL_RESPOND_CONFIG 3
#define SERIAL_RESPOND_EMPTY 4
#define SERIAL_RESPOND_ERROR 5
#define SERIAL_RESPOND_OK 6
#define SERIAL_CHANGE_CONFIG 7
#define SERIAL_REQUEST_RESET 8
#define SERIAL_IDENT_LED 9
#define SERIAL_IDENT_ENCODER 10
#define SERIAL_IDENT_BUTTON 11

#define COMMAND_CHAR '~'


class SerialMessage {
  public:
    SerialMessage() {}
    SerialMessage(char id, int len, char m_type) : id(id), length{len}, type{m_type} {}
    char id;
    int length; // Length of the data array
    char type; // Type as defined in serial.hpp
    char* data; // Array of data
};

/*
Process incoming serial data and fire the message handler when messages are encountered.
Place non-message bytes into the provided buffer

Pass 0 as outputBuffer to ignore non-message serial communication
*/
bool processSerial(void (*handler)(const SerialMessage&), char* outputBuffer, int& bufferLen, const int bufferSize);

// Send a message over serial to the host device. Generates a new ID for this message.
int sendSerialMessage(const char type, const int len, const char* msg);
// Send a message over serial to the host device
int sendSerialMessage(const char type, const char id, const int len, const char* msg);
// Send a message over serial to the host device
int sendSerialMessage(const char type, const char id);
// Send a message over serial to the host device. Generates a new ID for this message.
int sendSerialMessage(const char type);

// Reads the header bytes for a message from serial. Must be called after the message start byte has been read.
void receiveSerialMessageHeader(SerialMessage& msg);
// Reads message data from serial. Must be called after message header bytes have been read.
void receiveSerialMessageData(SerialMessage& msg);

// Blocks until Serial connects
void waitForSerial();

// Splits a 4 byte int into the provided buffer
void splitIntToBytes(const int number, char* bytes);

// Joins the provided 4 bytes into a single int
int joinBytesToInt(const char* bytes);


#endif