#ifndef profile_h
#define profile_h

#include <ArduinoJson.h>
#include "deck.hpp"


class Binding {
  public:
    Binding() {}
    Binding(const JsonObject& json, const HWDefinition& hw);
    int hwID;
};

class Profile {
  public:
    Profile() {}
    Profile(const JsonObject& json, const HWDefinition& hw);
    char* name;
    char r;
    char g;
    char b;
    Binding* bindings;
};


#endif
