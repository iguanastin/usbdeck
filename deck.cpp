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

HWRGBLight::HWRGBLight(const JsonObject& json) : HWOutput(json) {
  gPin = json["gpin"];
  bPin = json["bpin"];
  r = json["r"];
  g = json["g"];
  b = json["b"];
  pinMode(pin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
  analogWrite(pin, r);
  analogWrite(gPin, g);
  analogWrite(bPin, b);
}

HWButton::HWButton(const JsonObject& json) : HWInput(json) {
  detect = json["detect"];
  debounce = json["debounce"];
  button.setPressedState(detect);
  button.interval(debounce);
  button.attach(pin, INPUT_PULLUP);
}
bool HWButton::update() {
  bool result = button.update();
  if (button.pressed() && binding != NULL && binding->action1 != NULL) binding->action1->perform();
  else if (button.released() && binding != NULL && binding->action2 != NULL) binding->action2->perform();

  return result;
}

HWEncoder::HWEncoder(const JsonObject& json) : HWInput(json) {
  pin2 = json["pin2"];
  encoder = new Encoder(pin, pin2);
}
bool HWEncoder::update() {
  long delta = encoder->readAndReset();
  if (delta < 0 && binding != NULL && binding->action1 != NULL) binding->action1->perform();
  if (delta > 0 && binding != NULL && binding->action2 != NULL) binding->action2->perform();
  lastDelta = delta;

  return delta != 0;
}

HWDefinition::HWDefinition(const JsonObject& json) {
  auto comps = json["components"].as<JsonArray>();
  
  for (const JsonObject& j : comps) {  
    String type = j["type"];
    if (type.equals("led")) ledCount++;
    else if (type.equals("rgbled")) rgbCount++;
    else if (type.equals("encoder")) encoderCount++;
    else if (type.equals("button")) buttonCount++;
  }

  leds = new HWLEDLight[ledCount];
  rgbs = new HWRGBLight[rgbCount];
  encoders = new HWEncoder[encoderCount];
  buttons = new HWButton[buttonCount];

  int ledI = 0;
  int rgbI = 0;
  int encoderI = 0;
  int buttonI = 0;
  for (const JsonObject& j : comps) {
    String type = j["type"];
    if (type.equals("led")) {
      leds[ledI++] = HWLEDLight(j);
    } else if (type.equals("rgbled")) {
      rgbs[rgbI++] = HWRGBLight(j);
    } else if (type.equals("encoder")) {
      encoders[encoderI++] = HWEncoder(j);
    } else if (type.equals("button")) {
      buttons[buttonI++] = HWButton(j);
    }
  }
}
