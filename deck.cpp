#include "core_pins.h"
#include "deck.h"
#include <Arduino.h>

HWComponent::HWComponent(const JsonObject& json) {
  pin = json["pin"];
}

HWOutput::HWOutput(const JsonObject& json) : HWComponent(json) {
}

HWInput::HWInput(const JsonObject& json) : HWComponent(json) {
  String input = json["input_type"];
  if (input.equals("INPUT")) {
    type = INPUT;
  } else if (input.equals("INPUT_PULLUP")) {
    type = INPUT_PULLUP;
  } else if (input.equals("INPUT_PULLDOWN")) {
    type = INPUT_PULLDOWN;
  } else {
    type = -1;
  }
}

HWLEDLight::HWLEDLight(const JsonObject& json) : HWOutput(json) {
}
void HWLEDLight::init() {
  pinMode(pin, OUTPUT);
}

HWButton::HWButton(const JsonObject& json) : HWInput(json) {
  detect = json["detect"];
  debounce = json["debounce"];
}
void HWButton::init() {
  pinMode(pin, type);
}

HWEncoder::HWEncoder(const JsonObject& json) : HWInput(json) {
  pin2 = json["pin2"];
}
void HWEncoder::init() {
  pinMode(pin, type);
  pinMode(pin2, type);
}

HWDefinition::HWDefinition(const JsonObject& json) {
  auto comps = json["components"].as<JsonArray>();
  
  for (const JsonObject& j : comps) {  
    String type = j["type"];
    if (type.equals("led")) ledCount++;
    else if (type.equals("rotaryencoder")) encoderCount++;
    else if (type.equals("pushbutton")) buttonCount++;
  }

  leds = new HWLEDLight[ledCount];
  encoders = new HWEncoder[encoderCount];
  buttons = new HWButton[buttonCount];

  int ledI = 0;
  int encoderI = 0;
  int buttonI = 0;
  for (const JsonObject& j : comps) {
    String type = j["type"];
    if (type.equals("led")) {
      leds[ledI] = HWLEDLight(j);
      leds[ledI].init();
      ledI++;
    } else if (type.equals("rotaryencoder")) {
      encoders[encoderI] = HWEncoder(j);
      encoders[encoderI].init();
      encoderI++;
    } else if (type.equals("pushbutton")) {
      buttons[buttonI] = HWButton(j);
      buttons[buttonI].init();
      buttonI++;
    }
  }
}
