#pragma once

#include "nodes.hpp"

class Config;
class Profile {
  public:
    char* name;
    Config* config;
    Node** nodes;
    int size;
    
    Profile(const JsonObject& json, Config* config);
};
