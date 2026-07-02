#pragma once

#include "profile.hpp"

using namespace std;

class Config {
  public:
    static const int version = 1;
    int active;
    
    const Profile& activeProfile();
    const int profileCount();
    void update();
  private:
    Profile* profiles;
    int size;
};
