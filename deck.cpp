#include "deck.hpp"


HWComponent::HWComponent(const JsonObject& json) {
  pin = json["pin"];
}

HWOutput::HWOutput(const JsonObject& json) : HWComponent(json) {
}

HWInput::HWInput(const JsonObject& json) : HWComponent(json) {
}

HWLEDLight::HWLEDLight(const JsonObject& json) : HWOutput(json) {
  pinMode(pin, OUTPUT);
}

HWButton::HWButton(const JsonObject& json) : HWInput(json) {
  button.setPressedState(json["detect"]);
  button.attach(pin, INPUT_PULLUP);
  button.interval(json["debounce"]);
}

HWEncoder::HWEncoder(const JsonObject& json) : HWInput(json) {
  encoder = new Encoder(pin, json["pin2"]);
}

HWDefinition::HWDefinition(const JsonObject& json) {
  auto comps = json["components"].as<JsonArray>();
  
  for (const JsonObject& j : comps) {  
    String type = j["type"];
    if (type.equals("led")) ledCount++;
    else if (type.equals("encoder")) encoderCount++;
    else if (type.equals("button")) buttonCount++;
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
      leds[ledI++] = HWLEDLight(j);
    } else if (type.equals("encoder")) {
      encoders[encoderI++] = HWEncoder(j);
    } else if (type.equals("button")) {
      buttons[buttonI++] = HWButton(j);
    }
  }
}
