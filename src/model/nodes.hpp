#pragma once

#include <ArduinoJson.h>
#include <Bounce2.h>
#include <functional>
using namespace std;

class Port {
  public:
    const String& name;
    Port** connections;
    int size = 0;

    function<void()> onFire;
    function<void(bool)> onFireBool;
    function<void(int)> onFireInt;
    function<void(float)> onFireFloat;

    Port(const String& name);

    void fire();
    void fire(bool);
    void fire(int);
    void fire(float);
};

class Profile;
class Node {
  public:
    const String& name;
    Profile* profile;
    Port** inputs;
    Port** outputs;
    int numOutputs = 0;
    int numInputs = 0;

    static Node* fromJSON(const JsonObject& json, Profile* profile);

    virtual void update() {};
    Port* getOutput(const String& name);
    Port* getInput(const String& name);
  protected:
    Node(const JsonObject& json, Profile* profile);
};

class SwitchNode : public Node {
  public:
    int pin;
    Port pressed;
    Bounce2::Button button;

    SwitchNode(const JsonObject& json, Profile* profile);

    void update();
};

class KeyNode : public Node {
  public:
    int key;
    Port pressed;

    KeyNode(const JsonObject& json, Profile* profile);
};
