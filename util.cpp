#include "util.hpp"


bool strMatch(const char* str1, const char* str2, const int len) {
  for (int i = 0; i < len; i++) {
    if (str1[i] != str2[i]) return false;
  }

  return true;
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