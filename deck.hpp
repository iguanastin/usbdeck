#ifndef deck_h
#define deck_h

#include <ArduinoJson.h>

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
    void init();
};

class HWInput : public HWComponent {
  public:
    int type;
  protected:
    HWInput(const JsonObject& json);
    HWInput() {}
};

class HWButton : public HWInput {
  public:
    HWButton(const JsonObject& json);
    HWButton() {}
    int detect;
    int debounce;
    void init();
};

class HWEncoder : public HWInput {
  public:
    HWEncoder(const JsonObject& json);
    HWEncoder() {}
    int pin2;
    void init();
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