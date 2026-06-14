#pragma once

#include <ArduinoJson.h>
#include <Bounce2.h>

class Port {
  public:
    const char* name;
    Port(const char* name);
    void fire();
    void fire(bool);
    void fire(int);
    void fire(float);
};

class Profile;
class Node {
  public:
    char* name;
    Profile* profile;
    Port** inputs;
    Port** outputs;
    int numOutputs = 0;
    int numInputs = 0;
    Bounce2::Button button;

    static Node* fromJSON(const JsonObject& json, Profile* profile);

    virtual void update();
  protected:
    Node(const JsonObject& json, Profile* profile);
};

class SwitchNode : public Node {
  public:
    int pin;
    Port up;
    Port down;
    Port state;

    SwitchNode(const JsonObject& json, Profile* profile);

    void update();
};
