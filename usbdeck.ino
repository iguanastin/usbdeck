#include <Bounce2.h>
// https://github.com/thomasfredericks/Bounce2#
#include <ArduinoJson.h>
// https://arduinojson.org/
#include <LittleFS.h>
// https://github.com/PaulStoffregen/LittleFS

#include "serial.hpp"
#include "deck.hpp"

void(* resetTeensy) (void) = 0;


const char* configFilename = "config.json";

LittleFS_Program fs;

HWDefinition hw;


void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Calling Serial.begin is irrelevant for Teensy, as it is initialized before setup is even called.
  // Serial.begin(9600); // Baud rate is ignored for Teensy's serial  over USB
  waitForSerial();
  Serial.println("Started serial!");

  // Init LittleFS filesystem
  if (!fs.begin(100000)) { // 65536 bytes is minimum amount
    while(true) {
      Serial.println("FAILED TO START LittleFS");
      delay(5000);
    }
  }

  File cfgFile = fs.open(configFilename, FILE_READ);
  if (cfgFile) {
    readConfigFromFile(cfgFile);

    digitalWrite(LED_BUILTIN, LOW);
  } else {
    Serial.println("Failed to load config file");
  }
  cfgFile.close();

}

void loop() {

  // Handle inputs?

  doSerial();
  
}

bool writeStringToFile(const char* filepath, const char* bytes, const int length, const bool overwrite = true) {
  if (overwrite) fs.remove(filepath);
  File file = fs.open(filepath, FILE_WRITE_BEGIN);
  if (!file) return false;

  file.write(bytes, length);
  file.close();

  return true;
}

void doSerial() {
  const int bufferSize = 64; // 20 seems to be the maximum amount of bytes ever transferred in one USB packet
  int bufferLen = 0;
  char buffer[bufferSize];
  processSerial(&serialMessageHandler, buffer, bufferLen, bufferSize);

  for (int i = 0; i < bufferLen; i++) {
    if (buffer[i] == COMMAND_CHAR) {
      if (strMatch(buffer + i + 1, "reset", 5)) {
        Serial.println("Resetting Teensy, this may take a few seconds...");
        Serial.send_now();
        resetTeensy();
      } else if (strMatch(buffer + i + 1, "config", 6)) {
        File cfgFile = fs.open(configFilename, FILE_READ);
        if (cfgFile) {
          const int len = cfgFile.size();
          char str[len];
          cfgFile.readBytes(str, len);
          Serial.write(str, len);
          Serial.println();
        } else {
          Serial.println("Failed to open config file");
        }
        cfgFile.close();
      } else if (strMatch(buffer + i + 1, "clear", 5)) {
        if (fs.remove(configFilename)) {
          Serial.println("Deleted config file");
        } else {
          Serial.println("No config file/could not delete");
        }
      } else if (strMatch(buffer + i + 1, "sbuff", 5)) {
        Serial.write(buffer, bufferLen);
      }
    }
  }
}

void serialMessageHandler(const SerialMessage& msg) {
  // Handle request for the current config file
  if (msg.type == SERIAL_REQUEST_CONFIG) {
    File cfgFile = fs.open(configFilename, FILE_READ);
    if (!cfgFile) {
      const char* text = "No config file";
      sendSerialMessage(SERIAL_RESPOND_ERROR, msg.id, strlen(text), text);
      return;
    }

    int len = cfgFile.size();
    char data[len];
    cfgFile.readBytes(data, len);
    cfgFile.close();

    sendSerialMessage(SERIAL_RESPOND_CONFIG, msg.id, len, data);
  }

  // Host sent new config to apply
  else if (msg.type == SERIAL_CHANGE_CONFIG) {
    if (!writeStringToFile(configFilename, msg.data, msg.length)) {
      const char* text = "Failed to open config file for write";
      sendSerialMessage(SERIAL_RESPOND_ERROR, msg.id, strlen(text), text);
    }

    sendSerialMessage(SERIAL_RESPOND_OK, msg.id);
    Serial.send_now(); // Make sure the serial message was sent
    resetTeensy(); // Reset the device and load the new config
  }

  // Reset Teensy
  else if (msg.type == SERIAL_REQUEST_RESET) {
    sendSerialMessage(SERIAL_RESPOND_OK, msg.id);
    Serial.send_now();
    resetTeensy();
  }
}

void readConfigFromFile(File& cfgFile) {
  // Read config file
  int size = cfgFile.size();
  char jsonStr[size];
  cfgFile.readBytes(jsonStr, size);
  cfgFile.close();

  // Parse JSON config
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonStr, size);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  } else {
    // Create hardware definition
    hw = HWDefinition(doc.as<JsonObject>());
    doc.clear();
    doc.garbageCollect();
  }
}

bool strMatch(const char* str1, const char* str2, const int len) {
  for (int i = 0; i < len; i++) {
    if (str1[i] != str2[i]) return false;
  }

  return true;
}
