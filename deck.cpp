#include "deck.hpp"


HWComponent::HWComponent(const JsonObject& json) {
  pin = json["pin"];
  id = json["id"];
}

HWOutput::HWOutput(const JsonObject& json) : HWComponent(json) {
}

HWInput::HWInput(const JsonObject& json) : HWComponent(json) {
}

HWLEDLight::HWLEDLight(const JsonObject& json) : HWOutput(json) {
  pinMode(pin, OUTPUT);
}

HWButton::HWButton(const JsonObject& json) : HWInput(json) {
  detect = json["detect"];
  debounce = json["debounce"];
  button.setPressedState(detect);
  button.interval(debounce);
  button.attach(pin, INPUT_PULLUP);
}
bool HWButton::update() {
  if (binding == NULL) return false;

  bool result = button.update();
  if (button.pressed()) binding->action1->perform();
  else if (button.released()) binding->action2->perform();

  return result;
}

HWEncoder::HWEncoder(const JsonObject& json) : HWInput(json) {
  pin2 = json["pin2"];
  encoder = new Encoder(pin, pin2);
}
bool HWEncoder::update() {
  if (binding == NULL) return false;

  long delta = encoder->readAndReset();
  if (delta < 0) binding->action1->perform();
  if (delta > 0) binding->action2->perform();
  lastDelta = delta;

  return delta != 0;
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
