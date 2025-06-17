#include "core_pins.h"
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
  if (binding != NULL) {
    if (button.pressed() && binding->action1 != NULL) binding->action1->perform();
    else if (button.released() && binding->action2 != NULL) binding->action2->perform();
  }

  return result;
}

HWEncoder::HWEncoder(const JsonObject& json) : HWInput(json) {
  pin2 = json["pin2"];
  encoder = new Encoder(pin, pin2);
}
bool HWEncoder::update() {
  long delta = encoder->read();
  if (delta < 3 && delta > -3) return false; // Delta must be >= |3| to activate, or else it activates 4 times per detent
  encoder->readAndReset();

  if (binding != NULL) {
    if (delta < 0 && binding->action1 != NULL) binding->action1->perform();
    if (delta > 0 && binding->action2 != NULL) binding->action2->perform();
  }
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

void LEDIdent::update() {
  if (pin >= 0) {
    if (timer > length) {
      digitalWrite(pin, LOW);
      pin = -1;
    } else if (flashTimer > flashLength) {
      flashTimer = 0;
      digitalToggle(pin);
    }
  }
}
void LEDIdent::start(int ledPin) {
  pin = ledPin;
  timer = 0;
  flashTimer = 0;
  digitalWrite(pin, HIGH);
}

void RGBLEDIdent::update() {
  if (rPin > 0) {
    if (timer > length) {
      analogWrite(rPin, 0);
      analogWrite(gPin, 0);
      analogWrite(bPin, 0);
      rPin = -1;
      gPin = -1;
      bPin = -1;
    } else {
      analogWrite(rPin, (int)((sin(timer * 2 / 1000.0) + 1) * 255));
      analogWrite(gPin, (int)((sin(timer * 2 / 1000.0 + 1.05) + 1) * 255));
      analogWrite(bPin, (int)((sin(timer * 2 / 1000.0 + 2.1) + 1) * 255));
    }
  }
}
void RGBLEDIdent::start(int r, int g, int b) {
  rPin = r;
  gPin = g;
  bPin = b;
  timer = 0;
}
