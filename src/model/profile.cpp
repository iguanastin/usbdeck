#include "profile.hpp"


Profile::Profile(const JsonObject& json, Config* config) {
  this->config = config;
  name = json["name"];

  const JsonArray& n = json["nodes"];
  size = n.size();
  nodes = new Node*[size];

  int i = 0;
  for (const JsonObject& j: n) {
    nodes[i] = Node::fromJSON(j, this);
  }
}
