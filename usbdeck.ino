#include <Bounce2.h>
// https://github.com/thomasfredericks/Bounce2#
#include <ArduinoJson.h>
// https://arduinojson.org/
#include <LittleFS.h>
// https://github.com/PaulStoffregen/LittleFS

#include "serial.hpp"
#include "deck.hpp"
#include "profile.hpp"

void(* resetTeensy) (void) = 0;


const char* configFilename = "config.json";

LittleFS_Program fs;

HWDefinition hw;
bool configLoaded = false;
elapsedMillis errorLedTimer;

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Calling Serial.begin is irrelevant for Teensy, as it is initialized before setup is even called.
  // Serial.begin(9600); // Baud rate is ignored for Teensy's serial  over USB
  // waitForSerial();
  Serial.println("- USBDeck is starting up -");

  // Init LittleFS filesystem
  if (!fs.begin(1024*1024)) { // 131072 bytes seems to be required just to initialize LittleFS
    Serial.println("*** FAILED TO START LittleFS ***");
  }
  
  Serial.print(F("LittleFS Used Space: "));
  Serial.println(fs.usedSize());
  Serial.print(F("LittleFS Total Space: "));
  Serial.println(fs.totalSize());

  File cfgFile = fs.open(configFilename, FILE_READ);
  if (cfgFile) {
    configLoaded = readConfigFromFile(cfgFile);

    digitalWrite(LED_BUILTIN, LOW);
  } else {
    Serial.println("*** Failed to load config file ***");
  }
  cfgFile.close();

  Serial.println("- Finished startin USBDeck -");

}

void loop() {
  // Flash error LED if config wasn't loaded
  if (!configLoaded && errorLedTimer > 1000) {
    digitalToggle(LED_BUILTIN);
    errorLedTimer = 0;
  }

  // Handle inputs?

  doSerial();
  
}




bool writeStringToFile(const char* filepath, const char* bytes, const int length) {
  fs.remove(filepath);
  File file = fs.open(filepath, FILE_WRITE);
  if (!file) return false;

  file.write(bytes, length);
  file.close();
  if (file.getWriteError()) return false;

  return true;
}

void doSerial() {
  const int bufferSize = 64; // 20 seems to be the maximum amount of bytes ever transferred in one USB packet
  int bufferLen = 0;
  char buffer[bufferSize];
  processSerial(&serialMessageHandler, buffer, bufferLen, bufferSize);

  for (int i = 0; i < bufferLen; i++) {
    if (buffer[i] == COMMAND_CHAR) {
      if (strMatch(buffer + i + 1, "reset\n", 6)) {
        Serial.println("Resetting Teensy, this may take a few seconds...");
        Serial.send_now();
        resetTeensy();
      } else if (strMatch(buffer + i + 1, "config\n", 7)) {
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
      } else if (strMatch(buffer + i + 1, "clear\n", 6)) {
        if (fs.remove(configFilename)) {
          Serial.println("Deleted config file");
        } else {
          Serial.println("No config file/could not delete");
        }
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
      const char* text = "Failed to write config file";
      sendSerialMessage(SERIAL_RESPOND_ERROR, msg.id, strlen(text), text);
    } else {
      sendSerialMessage(SERIAL_RESPOND_OK, msg.id);
      Serial.send_now(); // Make sure the serial message was sent
      resetTeensy(); // Reset the device and load the new config
    }
  }

  // Reset Teensy
  else if (msg.type == SERIAL_REQUEST_RESET) {
    sendSerialMessage(SERIAL_RESPOND_OK, msg.id);
    Serial.send_now();
    resetTeensy();
  }

  // whoami
  else if (msg.type == SERIAL_REQUEST_IDENTIFY) {
    const char* text = "I'm a Teensy running USBDeck by iguanastin";
    sendSerialMessage(SERIAL_RESPOND_IDENTIFY, msg.id, strlen(text), text);
  }
}

bool readConfigFromFile(File& cfgFile) {
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
    return false;
  } else {
    // Create hardware definition
    hw = HWDefinition(doc["hardware"].as<JsonObject>());
    doc.clear();
    doc.garbageCollect();
    return true;
  }
}

bool strMatch(const char* str1, const char* str2, const int len) {
  for (int i = 0; i < len; i++) {
    if (str1[i] != str2[i]) return false;
  }

  return true;
}
