#ifndef deck_h
#define deck_h

#include <ArduinoJson.h>
#include <Bounce2.h>
// https://github.com/thomasfredericks/Bounce2#
#include <Encoder.h>
// https://github.com/PaulStoffregen/Encoder


class HWComponent {
  public:
    int pin;
  protected:
    HWComponent(const JsonObject& json);
    HWComponent() {}
};

class HWOutput : public HWComponent {
  protected:
    HWOutput(const JsonObject& json);
    HWOutput() {}
};

class HWLEDLight : public HWOutput {
  public:
    HWLEDLight(const JsonObject& json);
    HWLEDLight() {}
};

class HWInput : public HWComponent {
  protected:
    HWInput(const JsonObject& json);
    HWInput() {}
};

class HWButton : public HWInput {
  public:
    HWButton(const JsonObject& json);
    HWButton() {}
    Bounce2::Button button;
};

class HWEncoder : public HWInput {
  public:
    HWEncoder(const JsonObject& json);
    HWEncoder() {}
    Encoder* encoder;
};

class HWDefinition {
  public:
    HWDefinition(const JsonObject& json);
    HWDefinition() {}
    int ledCount = 0;
    int buttonCount = 0;
    int encoderCount = 0;
    HWLEDLight* leds;
    HWButton* buttons;
    HWEncoder* encoders;
};


#endif