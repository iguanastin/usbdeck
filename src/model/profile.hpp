#pragma once

#include "nodes.hpp"

class Config;
class Profile {
  public:
    const String& name;
    Config* config;
    Node** nodes;
    int size;
    
    Profile(const JsonObject& json, Config* config);

    Node* getNode(const String& name);
};
