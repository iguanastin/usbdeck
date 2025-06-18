#include "core_pins.h"
#include "deck.hpp"


HWComponent::HWComponent(const JsonObject& json) {
  pin = json["pin"];
  id = json["id"];
}

HWLEDLight::HWLEDLight(const JsonObject& json) : HWComponent(json) {
  pinMode(pin, OUTPUT);
}

HWRGBLight::HWRGBLight(const JsonObject& json) : HWComponent(json) {
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

HWButton::HWButton(const JsonObject& json) : HWComponent(json) {
  detect = json["detect"];
  debounce = json["debounce"];
  button.setPressedState(detect);
  button.interval(debounce);
  button.attach(pin, INPUT_PULLUP);
}
bool HWButton::update() {
  button.update();
  if (button.pressed()) {
    Serial.print(id);
    Serial.println("p");
  }
  else if (button.released()) {
    Serial.print(id);
    Serial.println("r");
  }

  return false;
}

HWEncoder::HWEncoder(const JsonObject& json) : HWComponent(json) {
  pin2 = json["pin2"];
  encoder = new Encoder(pin, pin2);
}
bool HWEncoder::update() {
  long delta = encoder->read();
  if (delta <= 3 && delta >= -3) return false; // This still isn't quite right
  encoder->readAndReset();

  if (delta < 0) {
    Serial.print(id);
    Serial.println("ccw");
  }
  if (delta > 0) {
    Serial.print(id);
    Serial.println("cw");
  }
  lastDelta = delta;

  return delta != 0;
}

HWDefinition::HWDefinition(const JsonObject& json) {
  auto comps = json["components"].as<JsonArray>();
  
  size = comps.size();
  components = new HWComponent*[size];

  int i = 0;
  for (const JsonObject& j : comps) {
    const String& type = j["type"];
    if (type.equals("led")) {
      components[i] = new HWLEDLight(j);
    } else if (type.equals("rgbled")) {
      components[i] = new HWRGBLight(j);
    } else if (type.equals("encoder")) {
      components[i] = new HWEncoder(j);
    } else if (type.equals("button")) {
      components[i] = new HWButton(j);
    }
    i++;
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
