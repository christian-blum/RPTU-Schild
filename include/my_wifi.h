#ifndef __MY_WIFI_H
#define __MY_WIFI_H

void wifi_setup();
void wifi_loop();
void wifi_clearPreferences();

extern char wifi_hostname[32];
extern char wifi_ssid[32];
extern char wifi_password[64];
extern byte mac[6];
extern IPAddress wifi_ip;

#endif