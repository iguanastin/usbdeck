#ifndef deck_h
#define deck_h

#include <ArduinoJson.h>
#include <Bounce2.h>
// https://github.com/thomasfredericks/Bounce2#
#include <Encoder.h>
// https://github.com/PaulStoffregen/Encoder


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

// LED light
class HWLEDLight : public HWComponent {
  public:
    HWLEDLight(const JsonObject& json);
    HWLEDLight() {}
};

// RGB LED Light
class HWRGBLight : public HWComponent {
  public:
    HWRGBLight(const JsonObject& json);
    HWRGBLight() {}
    int gPin;
    int bPin;
    int r;
    int g;
    int b;
};

// Button
class HWButton : public HWComponent {
  public:
    HWButton(const JsonObject& json);
    HWButton() {}
    int detect;
    int debounce;
    Bounce2::Button button;
    bool update();
};

// Rotary encoder
class HWEncoder : public HWComponent {
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
    int size = 0;
    HWComponent** components;
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