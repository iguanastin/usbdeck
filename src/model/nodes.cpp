#include "nodes.hpp"


void Port::fire() {
  onFire();
}
void Port::fire(bool b) {
  onFireBool(b);
  fire();
}
void Port::fire(int i) {
  onFireInt(i);
  fire();
}
void Port::fire(float f) {
  onFireFloat(f);
  fire();
}

void Port::fireConnections() {
  for (int i = 0; i < size; ++i) connections[i]->fire();
}
void Port::fireConnections(bool b) {
  for (int i = 0; i < size; ++i) connections[i]->fire(b);
}
void Port::fireConnections(int val) {
  for (int i = 0; i < size; ++i) connections[i]->fire(val);
}
void Port::fireConnections(bool f) {
  for (int i = 0; i < size; ++i) connections[i]->fire(f);
}

Node::Node(const JsonObject& json, Profile* profile) : name(json["name"]) {
  this->profile = profile;
}


Port* Node::getOutput(const String& name) {
  for (int i = 0; i < numOutputs; ++i) {
    if (name.equals(outputs[i]->name)) return outputs[i];
  }
  return nullptr;
}
Port* Node::getInput(const String& name) {
  for (int i = 0; i < numInputs; ++i) {
    if (name.equals(inputs[i]->name)) return inputs[i];
  }
  return nullptr;
}

Node* Node::fromJSON(const JsonObject& json, Profile* profile) {
  const String& type = json["type"];

  if (type.equals("switch")) return new SwitchNode(json, profile);
  else if (type.equals("key")) return new KeyNode(json, profile);

  return new Node(json, profile);
}

SwitchNode::SwitchNode(const JsonObject& json, Profile* profile): Node(json, profile),
                                                                  pin(json["pin"]),
                                                                  pressed(Port("Pressed")) {
  outputs = new Port*[]{&pressed};
  numOutputs = 1;

  pressed.onFireBool = [this](bool b) { pressed.fireConnections(b); };
  
  button.setPressedState(json["detect"] | true);
  button.interval(json["debounce"] | 5);
  button.attach(pin, INPUT_PULLUP);
}

void SwitchNode::update() {
  button.update();
  if (button.pressed()) pressed.fire(true);
  if (button.released()) pressed.fire(false);
}

KeyNode::KeyNode(const JsonObject& json, Profile* profile) : Node(json, profile),
                                                            key(json["key"]),
                                                            pressed(Port("Pressed")) {
  inputs = new Port*[]{&pressed};
  numInputs = 1;

  pressed.onFireBool = [this](bool b) {
    // TODO Press the key
  };
}


LEDNode::LEDNode(const JsonObject& json, Profile* profile) : Node(json, profile),
                                                            pin(json["pin"]),
                                                            percent(Port("Percent")) {
  inputs = new Port*[]{&percent};
  numInputs = 1;
  pinMode(pin, OUTPUT);
  analogWrite(pin, (int)(json["initial"].as<float>()*256));
  
  percent.onFireFloat = [this](float f) { analogWrite(pin, (int)(f*256)); };
}

RGBNode::RGBNode(const JsonObject& json, Profile* profile) : Node(json, profile), pin_r(json["pin_r"]), pin_g(json["pin_g"]), pin_b(json["pin_b"]), r(Port("R")), g(Port("G")), b(Port("B")) {
  inputs = new Port*[]{&r, &g, &b};
  numInputs = 3;
  pinMode(pin_r, OUTPUT);
  pinMode(pin_g, OUTPUT);
  pinMode(pin_b, OUTPUT);

  r.onFireFloat = [this](float f) { analogWrite(pin_r, (int)(f*256)); };
  g.onFireFloat = [this](float f) { analogWrite(pin_g, (int)(f*256)); };
  b.onFireFloat = [this](float f) { analogWrite(pin_b, (int)(f*256)); };
};
