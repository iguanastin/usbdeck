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

#define COMMAND_CHAR '~'


class SerialMessage {
  public:
    SerialMessage() {}
    SerialMessage(char id, int len, char m_type) : id(id), length{len}, type{m_type} {}
    char id;
    int length;
    char type;
    char* data;
};


bool processSerial(void (*handler)(const SerialMessage&), char* outputBuffer, int& bufferLen, const int bufferSize);

int sendSerialMessage(const char type, const int len, const char* msg);
int sendSerialMessage(const char type, const char id, const int len, const char* msg);
int sendSerialMessage(const char type, const char id);
int sendSerialMessage(const char type);

void receiveSerialMessageHeader(SerialMessage& msg);
void receiveSerialMessageData(SerialMessage& msg);

void waitForSerial();

void splitIntToBytes(const int number, char* bytes);

int joinBytesToInt(const char* bytes);


#endif