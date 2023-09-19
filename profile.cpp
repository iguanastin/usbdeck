#include "core_pins.h"
#include "usb_keyboard.h"
#include "keylayouts.h"
#include "usb_mouse.h"
#include "profile.hpp"
#include <Keyboard.h>
#include <Mouse.h>


Profile::Profile(const JsonObject& json) {
  const String& str = json["name"];
  const int len = str.length();
  name = new char[len + 1]();
  for (int i = 0; i < len; i++) {
    name[i] = str[i];
  }
  name[len] = '\0';

  r = json["r"].as<int>();
  g = json["g"].as<int>();
  b = json["b"].as<int>();

  const JsonArray& binds = json["bindings"].as<JsonArray>();
  bindingCount = binds.size();
  bindings = new Binding[bindingCount];
  for (int i = 0; i < bindingCount; i++) {
    bindings[i] = Binding(binds[i].as<JsonObject>());
  }
}

Binding::Binding(const JsonObject& json) {
  hwID = json["id"].as<int>();
  if (json.containsKey("action1")) action1 = parseAction(json["action1"].as<JsonObject>());
  if (json.containsKey("action2")) action2 = parseAction(json["action2"].as<JsonObject>());
}

StaticOutputBinding::StaticOutputBinding(const JsonObject& json) : Binding(json) { }

FlashLEDPattern::FlashLEDPattern(const JsonObject& json) : LEDPattern() {
  period = json["period"].as<int>();
}
void FlashLEDPattern::update() {
  if (timer > period) {
    timer = 0;
    digitalToggle(pin);
  }
}
void FlashLEDPattern::start(const int pin2) {
  pin = pin2;
  digitalWrite(pin, HIGH);
}

StaticLEDPattern::StaticLEDPattern(const JsonObject& json) : LEDPattern() {
  state = json["state"].as<bool>();
}
void StaticLEDPattern::start(const int pin) {
  if (state) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

PulseLEDPattern::PulseLEDPattern(const JsonObject& json) : LEDPattern() {
  period = json["period"].as<int>();
}
void PulseLEDPattern::start(const int pin2) {
  pin = pin2;
  timer = 0;
}
void PulseLEDPattern::update() {
  if (timer >= period) timer = 0;
  int state = (int)((sin(3.14*2 * timer/period) + 1) / 2 * 255);
  analogWrite(pin, state);
}

StaticLEDBinding::StaticLEDBinding(const JsonObject& json) : StaticOutputBinding(json) {
  if (json.containsKey("pattern")) {
    const JsonObject& patternJ = json["pattern"].as<JsonObject>();
    const int type = patternJ["type"].as<int>();
    if (type == LED_PATTERN_FLASH) pattern = new FlashLEDPattern(patternJ);
    else if (type == LED_PATTERN_STATIC) pattern = new StaticLEDPattern(patternJ);
    else if (type == LED_PATTERN_PULSE) pattern = new PulseLEDPattern(patternJ);
  }
}

void StaticLEDBinding::update() {
  if (pin >= 0) pattern->update();
}
void StaticLEDBinding::start() {
  if (pin >= 0) pattern->start(pin);
}

MouseAction::MouseAction(const JsonObject& json) : Action() {
  scrollY = json["scrolly"].as<int>();
  scrollX = json["scrollx"].as<int>();
  moveY = json["movey"].as<int>();
  moveX = json["movex"].as<int>();
  press = json["press"].as<bool>();
  release = json["release"].as<bool>();
  button = json["button"].as<int>();
}
void MouseAction::perform() {
  if (moveX != 0 || moveY != 0) Mouse.move(moveX, moveY);
  if (scrollX != 0 || scrollY != 0) Mouse.scroll(scrollY, scrollX);
  if (press) Mouse.press(button);
  if (release) Mouse.release(button);
}

KeyboardAction::KeyboardAction(const JsonObject& json) : Action() {
  if (json["ctrl"]) mods = mods | MODIFIERKEY_CTRL;
  if (json["shift"]) mods = mods | MODIFIERKEY_SHIFT;
  if (json["alt"]) mods = mods | MODIFIERKEY_ALT;
  if (json["gui"]) mods = mods | MODIFIERKEY_GUI;

  if (json.containsKey("keys")) {
    JsonArrayConst jsonKeys = json["keys"];
    for (unsigned int i = 0; i < jsonKeys.size() && i < 6; i++) {
      keys[i] = jsonKeys[i];
    }
  } else if (json.containsKey("print")) {
    print = new String(json["print"].as<String>());
  }
}
void KeyboardAction::perform() {
  if (print != NULL) {
    Keyboard.print(*print);
  } else {
    // TODO pressing one button, then pressing another before releasing the first will reset the held keys of the first binding
    // Probably want to use the individual press and release
    // Probably also need the action to be updated every loop to deal with necessary delays for instantaneous/hotkeys/multi key actions
    
    Keyboard.set_key1(0);
    Keyboard.set_key2(0);
    Keyboard.set_key3(0);
    Keyboard.set_key4(0);
    Keyboard.set_key5(0);
    Keyboard.set_key6(0);

    Keyboard.set_modifier(mods);
    Keyboard.send_now(); // Send modifiers first
    
    Keyboard.set_key1(keys[0]);
    Keyboard.set_key2(keys[1]);
    Keyboard.set_key3(keys[2]);
    Keyboard.set_key4(keys[3]);
    Keyboard.set_key5(keys[4]);
    Keyboard.set_key6(keys[5]);
    // Keyboard.set_media(uint16_t c); // TODO? This might not be supported.
    Keyboard.send_now();
  }
}

InstantKeyAction::InstantKeyAction(const JsonObject& json) : Action() {
  key = json["key"].as<int>();
}
void InstantKeyAction::perform() {
  Keyboard.press(key);
  Keyboard.release(key);
}

Action* parseAction(const JsonObject& json) {
  if (json == NULL) return NULL;

  const int type = json["type"];
  if (type == ACTION_MOUSE) return new MouseAction(json);
  if (type == ACTION_KEYBOARD) return new KeyboardAction(json);
  if (type == ACTION_INSTANT_KEY) return new InstantKeyAction(json);

  return NULL;
}