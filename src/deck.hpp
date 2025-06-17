#ifndef deck_h
#define deck_h

#include <ArduinoJson.h>
#include <Bounce2.h>
// https://github.com/thomasfredericks/Bounce2#
#include <Encoder.h>
// https://github.com/PaulStoffregen/Encoder
#include "profile.hpp"


// Basic definition of a hardware component
class HWComponent {
  public:
    int id; // Unique ID
    int pin; // Primary input/output pin
    virtual bool update() { return false; }
  protected:
    HWComponent(const JsonObject& json);
    HWComponent() {}
};

// Basic definition of a hardware output component
class HWOutput : public HWComponent {
  protected:
    HWOutput(const JsonObject& json);
    HWOutput() {}
};

// LED light
class HWLEDLight : public HWOutput {
  public:
    HWLEDLight(const JsonObject& json);
    HWLEDLight() {}
};

// RGB LED Light
class HWRGBLight : public HWOutput {
  public:
    HWRGBLight(const JsonObject& json);
    HWRGBLight() {}
    int gPin;
    int bPin;
    int r;
    int g;
    int b;
};

// Basic definition of a hardware input component
class HWInput : public HWComponent {
  public:
    Binding* binding = NULL;
  protected:
    HWInput(const JsonObject& json);
    HWInput() {}
};

// Button
class HWButton : public HWInput {
  public:
    HWButton(const JsonObject& json);
    HWButton() {}
    int detect;
    int debounce;
    Bounce2::Button button;
    bool update();
};

// Rotary encoder
class HWEncoder : public HWInput {
  public:
    HWEncoder(const JsonObject& json);
    HWEncoder() {}
    int pin2;
    int lastDelta = 0;
    Encoder* encoder;
    bool update();
};

// A complete hardware definition of all components
class HWDefinition {
  public:
    HWDefinition(const JsonObject& json);
    HWDefinition() {}
    int ledCount = 0;
    int rgbCount = 0;
    int buttonCount = 0;
    int encoderCount = 0;
    HWLEDLight* leds; // Array of LEDs
    HWRGBLight* rgbs; // Array of RGB LEDs
    HWButton* buttons; // Array of buttons
    HWEncoder* encoders; // Array of encoders
};

class LEDIdent {
  public:
    LEDIdent() {}
    LEDIdent(unsigned long int lengthMillis, unsigned long int flashMillis) {
      length = lengthMillis;
      flashLength = flashMillis;
    }
    void update();
    void start(int ledPin);
  private:
    unsigned long int length = 3000;
    unsigned long int flashLength = 250;
    elapsedMillis timer;
    elapsedMillis flashTimer;
    int pin = -1;
};

class RGBLEDIdent {
  public:
    RGBLEDIdent() {}
    RGBLEDIdent(unsigned long int lengthMillis) { length = lengthMillis; }
    void update();
    void start(int r, int g, int b);
  private:
    int rPin = -1;
    int gPin = -1;
    int bPin = -1;
    unsigned long int length = 3000;
    elapsedMillis timer;
};


#endif