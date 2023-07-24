#include "serial.hpp"

bool sendSerialRequest(const String& msg) {
  if (!Serial) return false;

  Serial.write(SERIAL_MESSAGE_START);

  char lenBytes[4];
  splitIntToBytes(msg.length() * sizeof(char), lenBytes);
  Serial.write(lenBytes, 4);

  Serial.print(msg);

  return true;
}

String receiveSerialMessage() {
  char lenBytes[4];
  Serial.readBytes(lenBytes, 4);
  const int length = joinBytesToInt(lenBytes);

  char result[length + 1];
  for (int i = 0; i < length; i++) {
    result[i] = Serial.read();
  }
  result[length] = '\0';
  
  return String(result);
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