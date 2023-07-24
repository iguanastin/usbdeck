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

  if (!fs.begin(100000)) { // 65536 bytes is minimum amount
    while(true) {
      Serial.println("FAILED TO START LittleFS");
      delay(5000);
    }
  }

  File cfgFile = fs.open(configFilename, FILE_READ);
  readConfigFromFile(cfgFile);
  cfgFile.close();

  digitalWrite(LED_BUILTIN, LOW);

}

void loop() {

  // Handle inputs?

  // Check for serial data?
  int bufferLen = 0;
  char buffer[128];
  processSerial(&serialMessageHandler, buffer, bufferLen);

  Serial.write(buffer, bufferLen); // Echo non-message serial back to the host
  
}

void serialMessageHandler(const SerialMessage& msg) {
  // Handle request for the current config file
  if (msg.type == SERIAL_REQUEST_CONFIG) {
    File cfgFile = fs.open(configFilename, FILE_READ);
    if (!cfgFile) {
      const char* text = "No config file";
      sendSerialMessage(SERIAL_RESPOND_ERROR, strlen(text), msg.id, text);
      return;
    }

    int len = cfgFile.size();
    char data[len];
    cfgFile.readBytes(data, len);
    cfgFile.close();

    sendSerialMessage(SERIAL_RESPOND_CONFIG, len, msg.id, data);
  }
}

void readConfigFromFile(File& cfgFile) {
  if (cfgFile) {
    digitalWrite(LED_BUILTIN, LOW);

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
  } else {
    Serial.println("Failed to load config file");
  }
}
