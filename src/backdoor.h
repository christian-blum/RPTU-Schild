#ifndef __BACKDOOR_H
#define __BACKDOOR_H

#include <Arduino.h>

#define PREF_NAMESPACE_BACKDOOR "backdoor"
#define PREF_BACKDOOR_USERNAME "username"
#define PREF_BACKDOOR_PASSWORD "password"

class Backdoor {

private:

public:
  Backdoor();
 ~Backdoor();

  const char *username;
  const char *password;

  void setup();
  bool configured();
  bool backdoor_authenticated();
  void uri_handler();

};

#endif
