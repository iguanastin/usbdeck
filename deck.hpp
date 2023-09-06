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
    virtual bool update() { return false; }
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


#endif