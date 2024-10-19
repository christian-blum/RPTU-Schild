#ifndef __MY_WEBSERVER_H
#define __MY_WEBSERVER_H

#include <Arduino.h>
#include <WebServer.h>

#include "my_wifi.h"



extern WebServer webserver;
extern void (*webserver_handle_root)();

bool webserver_admin_auth();
#define WEBSERVER_ADMIN(what) if (webserver_admin_auth()) { what }
String webserver_quote_special(String str);

void webserver_setup();
void webserver_loop();
void webserver_send_redirect(String location);
void webserver_clearPreferences();

#endif