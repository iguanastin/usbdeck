#include "elapsedMillis.h"
#ifndef profile_h
#define profile_h

#include <ArduinoJson.h>

/*

single key up and down
single key instantaneous
single key repeat while down
multi key
mouse click up and down
mouse click instant
mouse click repeatedly while down
mouse scroll
mouse scroll while down
mouse move
mouse move while down
mouse position
led flash
led flash while down
led hold while down
rgb single color
rgb pattern
rgb pattern while down
desktop action (requires driver/serial communicator)
multiple


key params:
  type
  delays
  key ids
  modifiers
mouse params:
  type
  delays
  scroll
  move
  position
  button id
led params:
  type
  pattern
  on/off/pwm

activations:
  button:
    press
    release
  encoder:
    left
    right

*/

#define ACTION_MOUSE 1
#define ACTION_KEYBOARD 2
#define ACTION_INSTANT_KEY 3

#define LED_PATTERN_FLASH 1
#define LED_PATTERN_STATIC 2
#define LED_PATTERN_PULSE 3


class Action {
  public:
    Action() {}
    virtual void perform() = 0;
};

class Binding {
  public:
    Binding(const JsonObject& json);
    Binding() {}
    int hwID;
    Action* action1; // TODO split actions into another subclass (ActionBinding)
    Action* action2;
    virtual void update() {}
    virtual void start() {}
};

class StaticOutputBinding : public Binding {
  public:
    StaticOutputBinding(const JsonObject& json);
};

class LEDPattern { 
  public:
    elapsedMillis timer;
    int pin;
    virtual int type() { return 0; }
    virtual void start(const int pin) {}
    virtual void update() {}
};

class FlashLEDPattern : public LEDPattern {
  public:
    FlashLEDPattern(unsigned long int periodMillis) { period = periodMillis; }
    FlashLEDPattern(const JsonObject& json);
    unsigned long int period = 100;
    void start(const int pin);
    void update();
    int type() { return LED_PATTERN_FLASH; }
};

class StaticLEDPattern : public LEDPattern {
  public:
    StaticLEDPattern(bool on) { state = on; }
    StaticLEDPattern(const JsonObject& json);
    bool state;
    void start(const int pin);
    int type() { return LED_PATTERN_STATIC; }
};

class PulseLEDPattern : public LEDPattern {
  public:
    PulseLEDPattern(unsigned long int periodMillis) { period = periodMillis; }
    PulseLEDPattern(const JsonObject& json);
    unsigned long int period;
    void start(const int pin);
    void update();
    int type() { return LED_PATTERN_PULSE; }
};

class StaticLEDBinding : public StaticOutputBinding {
  public:
    StaticLEDBinding(const JsonObject& json);
    LEDPattern* pattern;
    int pin = -1;
    void update();
    void start();
};

class MouseAction : public Action {
  public:
    MouseAction(const JsonObject& json);
    int button = 0;
    bool press = false;
    bool release = false;
    int scrollX = 0;
    int scrollY = 0;
    int moveX = 0;
    int moveY = 0;
    void perform();
};

class KeyboardAction : public Action {
  public:
    KeyboardAction(const JsonObject& json);
    int keys[6];
    int mods = 0;
    String* print;
    void perform();
};

class InstantKeyAction : public Action {
  public:
    InstantKeyAction(const JsonObject& json);
    int key;
    void perform();
};

class Profile {
  public:
    Profile() {}
    Profile(const JsonObject& json);
    char* name;
    char r = 255;
    char g = 255;
    char b = 255;
    int bindingCount = 0;
    Binding* bindings;
};


Action* parseAction(const JsonObject& json);


#endif
