#include <ArduinoJson.h>
// https://arduinojson.org/
#include <LittleFS.h>
// https://github.com/PaulStoffregen/LittleFS

#include "serial.hpp"
#include "deck.hpp"
#include "profile.hpp"

#define JSON_DOC_MAX_SIZE 8192 // Probably overkill for most configurations. Really complex ones might need a higher max
#define LITTLE_FS_SIZE 1048576 // Minimum of 131072 bytes seems to be required just to initialize LittleFS


void(* resetTeensy) (void) = 0; // Suspicious software reset that probably doesn't cycle memory and leaks everything instead


const char* configFilename = "config.json"; // Filename/path to the config file

LittleFS_Program fs; // File store

HWDefinition hw; // Hardware definition (buttons, encoders, lights, etc.)
bool configLoaded = false; // Is true after a config successfully loads

elapsedMillis errorLedTimer; // LED flash timer for catastrophic errors
elapsedMillis ledIdentTimer; // LED timer for identifying different LEDs
elapsedMillis ledRGBIdentTimer;
elapsedMillis ledIdentFlashTimer; // LED flash timer for identifying different LEDs
int ledIdentPin = -1; // Current LED pin to flash, or -1 to not flash any
int ledRIdentPin = -1;
int ledGIdentPin = -1;
int ledBIdentPin = -1;

bool identMode = true; // If board is in ident mode, inputs will send an ident command to the configurator instead of performing default config binding actions


void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT); // Built-in LED init
  digitalWrite(LED_BUILTIN, HIGH);

  // Calling Serial.begin is irrelevant for Teensy, as it is initialized before setup is even called.
  // Serial.begin(9600); // Baud rate is ignored for Teensy's serial  over USB
  // waitForSerial();
  Serial.println(F("- USBDeck is starting up -"));

  // Init LittleFS filesystem
  if (!fs.begin(LITTLE_FS_SIZE)) {
    Serial.println(F("*** FAILED TO START LittleFS ***"));
  }

  #ifdef ARDUINO_TEENSY41
  Serial.println(F("Teensy 4.1"));
  #else
  Serial.println(F("*** UNEXPECTED HARDWARE ***"));
  #endif
  
  Serial.print(F("LittleFS Used Space: "));
  Serial.println(fs.usedSize());
  Serial.print(F("LittleFS Total Space: "));
  Serial.println(fs.totalSize());

  if (!readConfig()) {
    Serial.println(F("*** Failed to load/apply config ***"));
  }

  Serial.println(F("- Finished startin USBDeck -"));

  digitalWrite(LED_BUILTIN, LOW); // Turn off built-in LED after setup finishes

}

void loop() {
  // Flash error LED if config wasn't loaded
  if (!configLoaded && errorLedTimer > 1000) {
    digitalToggle(LED_BUILTIN);
    errorLedTimer = 0;
  }
  // Disable ident LED after timer
  if (ledIdentPin >= 0) {
    if (ledIdentTimer > 3000) {
      digitalWrite(ledIdentPin, LOW);
      ledIdentPin = -1;
    } else if (ledIdentFlashTimer > 250) {
      ledIdentFlashTimer = 0;
      digitalToggle(ledIdentPin);
    }
  }
  if (ledRIdentPin >= 0) {
    if (ledRGBIdentTimer > 3000) {
      analogWrite(ledRIdentPin, 0);
      analogWrite(ledGIdentPin, 0);
      analogWrite(ledBIdentPin, 0);
      ledRIdentPin = -1;
      ledGIdentPin = -1;
      ledBIdentPin = -1;
    } else {
      analogWrite(ledRIdentPin, (int)((sin(ledRGBIdentTimer * 2 / 1000.0) + 1) * 255));
      analogWrite(ledGIdentPin, (int)((sin(ledRGBIdentTimer * 2 / 1000.0 + 1.05) + 1) * 255));
      analogWrite(ledBIdentPin, (int)((sin(ledRGBIdentTimer * 2 / 1000.0 + 2.1) + 1) * 255));
    }
  }


  // Handle inputs?
  updateInputs();

  // Handle serial
  doSerial();
  
}



// Check hardware for events
void updateInputs() {
  // Update buttons
  for (int i = 0; i < hw.buttonCount; i++) {
    HWButton& btn = hw.buttons[i];
    Binding* bind = btn.binding;

    if (identMode) btn.binding = NULL; // TODO Probably a terrible way to make the button not perform the bound action when identifying
    if (btn.update() && identMode) {
      Serial.println("IDENT");
      identButton(btn);
    }
    if (identMode) btn.binding = bind; // TODO Probably a terrible way to make the button not perform the bound action when identifying
  }
  for (int i = 0; i < hw.encoderCount; i++) {
    HWEncoder& enc = hw.encoders[i];
    Binding* bind = enc.binding;

    if (identMode) enc.binding = NULL; // TODO Probably a terrible way to make the button not perform the bound action when identifying
    if (enc.update() && identMode) {
      identEncoder(enc, enc.lastDelta);
    }
    if (identMode) enc.binding = bind; // TODO Probably a terrible way to make the button not perform the bound action when identifying
  }
}

// Send ident request for a specified encoder
void identEncoder(const HWEncoder& encoder, int delta) {
  char pinBytes[4];
  int pin = encoder.pin;
  if (delta > 0) pin = encoder.pin2;
  splitIntToBytes(pin, pinBytes);
  sendSerialMessage(SERIAL_IDENT_ENCODER, 4, pinBytes);
}

// Send ident request for a specified button
void identButton(const HWButton& button) {
  char pinBytes[4];
  splitIntToBytes(button.pin, pinBytes);
  sendSerialMessage(SERIAL_IDENT_BUTTON, 4, pinBytes);
}

// Utility function to write a byte array to a filepath
bool writeStringToFile(const char* filepath, const char* bytes, const int length) {
  fs.remove(filepath);
  File file = fs.open(filepath, FILE_WRITE);
  if (!file) return false;

  file.write(bytes, length);
  file.close();
  if (file.getWriteError()) return false;

  return true;
}

// Read serial and process it, responding to received messages as necessary
void doSerial() {
  const int bufferSize = 64; // 20 seems to be the maximum amount of bytes ever transferred in one USB packet
  int bufferLen = 0;
  char buffer[bufferSize];
  // Read available serial and check for messages
  processSerial(&serialMessageHandler, buffer, bufferLen, bufferSize);

  // Check for serial user commands
  for (int i = 0; i < bufferLen; i++) {
    if (buffer[i] == COMMAND_CHAR) {
      if (strMatch(buffer + i + 1, "help\n", 5)) {
        Serial.println(F("~help    - Display available commands"));
        Serial.println(F("~reset   - Soft resets this device"));
        Serial.println(F("~config  - Display the config JSON currently being used"));
        Serial.println(F("~clear   - Clear the config of this device and do a soft reset"));
        Serial.println(F("~fsstat  - Display filesystem usage"));
        Serial.println(F("~hwstat  - Display hardware component counts"));
      } else if (strMatch(buffer + i + 1, "reset\n", 6)) {
        Serial.println(F("Resetting Teensy, this may take a few seconds..."));
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
          Serial.println(F("Failed to open config file"));
        }
        cfgFile.close();
      } else if (strMatch(buffer + i + 1, "clear\n", 6)) {
        if (fs.remove(configFilename)) {
          Serial.println(F("Deleted config file"));
        } else {
          Serial.println(F("No config file/could not delete"));
        }
      } else if (strMatch(buffer + i + 1, "fsstat\n", 7)) {
        Serial.print(F("FS Used: "));
        Serial.println(fs.usedSize());
        Serial.print(F("FS Total: "));
        Serial.println(fs.totalSize());
      } else if (strMatch(buffer + i + 1, "hwstat\n", 7)) {
        Serial.print(F("Buttons: "));
        Serial.println(hw.buttonCount);
        Serial.print(F("Encoders: "));
        Serial.println(hw.encoderCount);
        Serial.print(F("LEDs: "));
        Serial.println(hw.ledCount);
      }
    }
  }
}

// Handles all incoming serial messages
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

  // Ident LED
  else if (msg.type == SERIAL_IDENT_LED) {
    ledIdentPin = 0;
    for (int i = 0; i < msg.length; i++) {
      ledIdentPin = ledIdentPin * 10 + (msg.data[i] - '0');
    }
    ledIdentTimer = 0;
    digitalWrite(ledIdentPin, HIGH);

    sendSerialMessage(SERIAL_RESPOND_OK, msg.id);
  }

  else if (msg.type == SERIAL_IDENT_RGB) {
    ledRIdentPin = joinBytesToInt(msg.data);
    ledGIdentPin = joinBytesToInt(msg.data+4);
    ledBIdentPin = joinBytesToInt(msg.data+8);
    ledRGBIdentTimer = 0;

    sendSerialMessage(SERIAL_RESPOND_OK, msg.id);
  }
}

// Read config file and apply
bool readConfig() {
  File cfgFile = fs.open(configFilename, FILE_READ);
  if (cfgFile) {
    configLoaded = readConfigFromFile(cfgFile);
  } else {
    Serial.println(F("*** Failed to read config file ***"));
  }
  cfgFile.close();

  return configLoaded;
}

// Read and apply hardware/bindings/profiles configuration file
bool readConfigFromFile(File& cfgFile) {
  // Read config file
  int size = cfgFile.size();
  char jsonStr[size];
  cfgFile.readBytes(jsonStr, size);
  cfgFile.close();

  // Parse JSON config
  DynamicJsonDocument doc(JSON_DOC_MAX_SIZE);
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

// Check if two char arrays exactly match for a specified length
bool strMatch(const char* str1, const char* str2, const int len) {
  for (int i = 0; i < len; i++) {
    if (str1[i] != str2[i]) return false;
  }

  return true;
}
