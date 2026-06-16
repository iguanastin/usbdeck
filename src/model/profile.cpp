#include "profile.hpp"


Profile::Profile(const JsonObject& json, Config* config) : name(json["name"]) {
  this->config = config;

  const JsonArray& n = json["nodes"];
  size = n.size();
  nodes = new Node*[size];

  int i = 0;
  for (const JsonObject& j: n) {
    nodes[i++] = Node::fromJSON(j, this);
  }

  // Connect node output ports to input ports.
  for (const JsonObject& nodej : n) {
    Node& from = *getNode(nodej["name"]);
    for (const JsonObject& portj : nodej["outputs"].as<JsonArray>()) {
      Port& fromPort = *from.getOutput(portj["name"]);
      
      fromPort.size = portj["to"].size();
      fromPort.connections = new Port*[fromPort.size];
      int i = 0;
      for (const JsonObject& connj : portj["to"].as<JsonArray>()) {
        Port& toPort = *getNode(connj["to_node"])->getInput(connj["to_port"]);

        fromPort.connections[i++] = &toPort;
      }
    }
  }
}

Node* Profile::getNode(const String& name) {
  for (int i = 0; i < size; ++i) {
    if (name.equals(nodes[i]->name)) return nodes[i];
  }
  return nullptr;
}
