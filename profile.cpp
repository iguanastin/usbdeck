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
}

Binding::Binding(const JsonObject& json) {
  hwID = json["id"];
  if (json.containsKey("action1")) action1 = parseAction(json["action1"]);
  if (json.containsKey("action2")) action2 = parseAction(json["action2"]);
}

MouseAction::MouseAction(const JsonObject& json) : Action() {
  if (json.containsKey("scrolly")) scrollY = json["scrolly"];
  if (json.containsKey("scrollx")) scrollX = json["scrollx"];
  if (json.containsKey("movey")) moveY = json["movey"];
  if (json.containsKey("movex")) moveX = json["movex"];
  if (json.containsKey("press")) press = json["press"];
  if (json.containsKey("release")) release = json["release"];
  if (json.containsKey("button")) button = json["button"];
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

Action* parseAction(const JsonObject& json) {
  const int type = json["type"];
  if (type == ACTION_MOUSE) return new MouseAction(json);
  if (type == ACTION_KEYBOARD) return new KeyboardAction(json);

  return NULL;
}