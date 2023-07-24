#include <Bounce2.h>
// https://github.com/thomasfredericks/Bounce2#
#include <ArduinoJson.h>
// https://arduinojson.org/
#include <LittleFS.h>
// https://github.com/PaulStoffregen/LittleFS

#include "deck.h"
#include "serial.h"

void(* resetTeensy) (void) = 0;


LittleFS_Program fs;

HWDefinition hw;

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Calling Serial.begin is irrelevant for Teensy, as it is initialized before setup is even called.
  // Serial.begin(9600); // Baud rate is ignored for Teensy's serial  over USB
  waitForSerial();
  Serial.println("Started serial!");

  fs.begin(65536); // 65536 bytes is minimum amount
  File cfgFile = fs.open("config.json", FILE_READ);

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

void loop() {

  // Handle inputs?

  // Check for serial data?
  while (Serial.available()) {
    int read = Serial.read();
    if (read == '@') Serial.println("You sent an @!");
    if (read == '~') {
      Serial.println("Restarting Teensy, this may take a few seconds...");
      Serial.send_now();
      resetTeensy();
    }
    if (read == '!') {
      Keyboard.print("test");
    }
    if (read == SERIAL_MESSAGE_START) {
      auto msg = receiveSerialMessage();
      // TODO do something with the message
    }
  }
  
}
