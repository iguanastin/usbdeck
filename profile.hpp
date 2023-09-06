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


class Action {
  public:
    Action() {}
    virtual void perform();
};

class Binding {
  public:
    Binding(const JsonObject& json);
    Binding() {}
    int hwID;
    Action* action1;
    Action* action2;
    virtual void update();
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
