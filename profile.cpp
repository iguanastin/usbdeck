#include "profile.hpp"


Profile::Profile(const JsonObject& json, const HWDefinition& hw) {
  const char* str = json["name"];
  const int len = strlen(str);
  name = new char[len + 1]();
  for (int i = 0; i < len; i++) {
    name[i] = str[i];
  }
  name[len] = '\0';

  r = json["r"].as<int>();
  g = json["g"].as<int>();
  b = json["b"].as<int>();
}

Binding::Binding(const JsonObject& json, const HWDefinition& hw) {
  // TODO
}