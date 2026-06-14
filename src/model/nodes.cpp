#include "nodes.hpp"


Node::Node(const JsonObject& json, Profile* profile) {
  this->profile = profile;
  name = json["name"];
  // TODO inputs/outputs
}

Node* Node::fromJSON(const JsonObject& json, Profile* profile) {
  const String& type = json["type"];

  if (type.equals("switch")) return new SwitchNode(json, profile);

  return new Node(json, profile);
}


SwitchNode::SwitchNode(const JsonObject& json, Profile* profile): Node(json, profile), 
                                                                  up(Port("Up")), 
                                                                  down(Port("Down")), 
                                                                  state(Port("State")), 
                                                                  pin(json["pin"]) {
  outputs = new Port*[]{&up, &down, &state};
  numOutputs = 3;
  
  button.setPressedState(json["detect"] | true);
  button.interval(json["debounce"] | 5);
  button.attach(pin, INPUT_PULLUP);
}

void SwitchNode::update() {
  button.update();
  if (button.pressed()) {
    down.fire();
    state.fire(true);
  }
  if (button.released()) {
    up.fire();
    state.fire(false);
  }
}