#include "serial.hpp"


char requestIDCounter = 1;  // Need to upgrade this to int if a lot of messages are sent
// If both host and device are sending messages with their own id counter, there might be collisions


// Pass 0 as outputBuffer to ignore non-message serial communication
bool processSerial(void (*msgHandler)(const SerialMessage&), char* outputBuffer, int& bufferLen, const int bufferSize) {
  if (!Serial) return false;

  while (Serial.available()) {
    char read = Serial.peek();
    if (read == SERIAL_MESSAGE_START) {
      Serial.read();  // Pop the byte, it's important
      SerialMessage msg;
      receiveSerialMessageHeader(msg);
      if (msg.length > 0) {
        msg.data = new char[msg.length]();
        receiveSerialMessageData(msg);
      }
      msgHandler(msg);
      if (msg.length > 0) delete[] msg.data;
    } else if (outputBuffer) {
      // Write byte to the standard serial buffer (not a message byte)
      if (bufferLen < bufferSize) {
        Serial.read();  // Pop the byte, we're using it
        outputBuffer[bufferLen++] = read;
      } else {
        return true;
      }
    }
  }

  return true;
}


int sendSerialMessage(const char type, const char id) {
  return sendSerialMessage(type, id, 0, (const char*)0);
}

int sendSerialMessage(const char type) {
  return sendSerialMessage(type, 0, (const char*)0);
}

int sendSerialMessage(const char type, const int len, const char* msg) {
  return sendSerialMessage(type, requestIDCounter++, len, msg);
}

int sendSerialMessage(const char type, const char id, const int len, const char* msg) {
  if (!Serial) return 0;
  if (requestIDCounter == 0) requestIDCounter++;

  // Send start byte
  Serial.write(SERIAL_MESSAGE_START);

  // Send type byte
  Serial.write(type);
  // Send id byte
  Serial.write(id);

  // Send 4 length bytes
  char lenBytes[4];
  splitIntToBytes(len * sizeof(char), lenBytes);
  Serial.write(lenBytes, 4);

  // Send message bytes
  if (len > 0) Serial.write(msg, len);

  return id;
}

void receiveSerialMessageHeader(SerialMessage& msg) {
  msg.type = Serial.read();
  msg.id = Serial.read();

  char lenBytes[4];
  Serial.readBytes(lenBytes, 4);
  msg.length = joinBytesToInt(lenBytes);
}

void receiveSerialMessageData(SerialMessage& msg) {
  Serial.readBytes(msg.data, msg.length);
}

void waitForSerial() {
  while (!Serial) continue;
}

void splitIntToBytes(const int number, char* bytes) {
  bytes[0] = (number & 0xff000000UL) >> 24;
  bytes[1] = (number & 0x00ff0000UL) >> 16;
  bytes[2] = (number & 0x0000ff00UL) >> 8;
  bytes[3] = (number & 0x000000ffUL);
}

int joinBytesToInt(const char* bytes) {
  return ((int)bytes[0] << 24) | ((int)bytes[1] << 16) | ((int)bytes[2] << 8) | (int)bytes[3];
}