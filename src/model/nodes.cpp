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

  pressed.onFireBool = [this](bool b) {
    for (int i = 0; i < pressed.size; ++i) {
      pressed.connections[i]->fire(b);
    }
  };
  
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
