#ifndef __BACKDOOR_H
#define __BACKDOOR_H

#include "defaults.h"

#ifdef HAVE_BACKDOOR

#include <Arduino.h>

#define PREF_NAMESPACE_BACKDOOR "backdoor"
#define PREF_BACKDOOR_USERNAME "username"
#define PREF_BACKDOOR_PASSWORD "password"

class Backdoor {

private:
  bool authenticate();

public:
  Backdoor();
 ~Backdoor();

  const char *username;
  const char *password;

  void setup();
  bool configured();
  bool authenticated();
  void uri_handler();

};

#endif

#endif
