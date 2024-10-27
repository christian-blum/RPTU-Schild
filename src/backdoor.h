#ifndef __BACKDOOR_H
#define __BACKDOOR_H

#include <Arduino.h>

#define PREF_NAMESPACE_BACKDOOR "backdoor"
#define PREF_BACKDOOR_USERNAME "username"
#define PREF_BACKDOOR_PASSWORD "password"
#define URI_BACKDOOR "/backdoor"

class Backdoor {

private:
  bool Backdoor::backdoor_authenticated();
  void handler_backdoor();

public:
  Backdoor();

  const char *username;
  const char *password;

  void setup();
  bool configured();

};

#endif
