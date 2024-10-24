#define WIFI_PREFS_NAMESPACE "net"
#define WIFI_PREFS_HOSTNAME "host"
#define WIFI_PREFS_SSID "ssid"
#define WIFI_PREFS_PASSWORD "pass"

#define WIFI_CONNECTION_TIMEOUT 30000

#include <WiFi.h>
#include <ESPmDNS.h>
#include "my_webserver.h"

#include <Preferences.h>


#define URI_NETCONFIG "/netconfig"


enum {
  WIFI_LOOP_STATE_IDLE = 0,
  WIFI_LOOP_STATE_INIT,
  WIFI_LOOP_STATE_WAITING_FOR_CONNECTION,
  WIFI_LOOP_STATE_CONNECTION_ESTABLISHED,
  WIFI_LOOP_STATE_CONNECTED,
  WIFI_LOOP_STATE_AP_MODE
} wifi_loop_state;
unsigned long wifi_initialization_time;

char wifi_hostname[32];
char wifi_ssid[32];
char wifi_password[64];
byte mac[6];
IPAddress wifi_ip;

void (*handler_connected)();
void (*handler_ap_mode)();


void wifi_set_handler_connected(void(*handler)()) {
  handler_connected = handler;
}

void wifi_set_handler_ap_mode(void(*handler)()) {
  handler_ap_mode = handler;
}

void wifi_clear_settings() {
  Preferences prefs;
  prefs.begin(WIFI_PREFS_NAMESPACE);
  prefs.clear();
  prefs.end();
  wifi_loop_state = WIFI_LOOP_STATE_INIT;
}

void wifi_readPreferences() {
  Preferences prefs;
  prefs.begin(WIFI_PREFS_NAMESPACE, true);
  if (!prefs.getString(WIFI_PREFS_HOSTNAME, wifi_hostname, sizeof(wifi_hostname))) {
    long unsigned int espmac = ESP.getEfuseMac() >> 24;
    snprintf(wifi_hostname, sizeof(wifi_hostname), "ESP32-%06lX", espmac);
  }
  if (!prefs.getString(WIFI_PREFS_SSID, wifi_ssid, sizeof(wifi_ssid))) wifi_ssid[0] = 0;
  if (!prefs.getString(WIFI_PREFS_PASSWORD, wifi_password, sizeof(wifi_password))) wifi_password[0] = 0;
  prefs.end();
}

void wifi_writePreferences() {
  Preferences prefs;
  prefs.begin(WIFI_PREFS_NAMESPACE);
  prefs.putString(WIFI_PREFS_HOSTNAME, wifi_hostname);
  prefs.putString(WIFI_PREFS_SSID, wifi_ssid);
  prefs.putString(WIFI_PREFS_PASSWORD, wifi_password);
  prefs.end();
}

void wifi_clearPreferences() {
  Preferences prefs;
  prefs.begin(WIFI_PREFS_NAMESPACE);
  prefs.clear();
  prefs.end();
}

struct sWifiNetwork {
  char ssid[32];
  wifi_auth_mode_t encryptionType;
  uint16_t accessPoints;
  int32_t bestRSSI;
  struct sWifiNetwork *next;
} *wifi_networks = NULL;

const char *wifi_enctype2String(wifi_auth_mode_t encType) {
  const char *ets;
  switch (encType) {
    case WIFI_AUTH_OPEN:            ets = "open"; break;
    case WIFI_AUTH_WEP:             ets = "WEP"; break;
    case WIFI_AUTH_WPA_PSK:         ets = "WPA"; break;
    case WIFI_AUTH_WPA2_PSK:        ets = "WPA2"; break;
    case WIFI_AUTH_WPA_WPA2_PSK:    ets = "WPA+WPA2"; break;
    case WIFI_AUTH_WPA2_ENTERPRISE: ets = "WPA2-EAP"; break;
    case WIFI_AUTH_WPA3_PSK:        ets = "WPA3"; break;
    case WIFI_AUTH_WPA2_WPA3_PSK:   ets = "WPA2+WPA3"; break;
    case WIFI_AUTH_WAPI_PSK:        ets = "WAPI"; break;
    default:                        ets = "unknown";
  }
  return ets;
}


void wifi_clear_scan_results() {
  struct sWifiNetwork *x, *y;
  x = wifi_networks;
  while (x) {
    y = x;
    x = y->next;
    free(y);
  }
  wifi_networks = NULL;
}

void wifi_scan() {
  struct sWifiNetwork *x, *y;
  wifi_clear_scan_results();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  uint16_t count = WiFi.scanNetworks(false, false, false, 500, 0);
  while (!WiFi.scanComplete()) {}
  for (uint16_t i = 0; i < count; i++) {
    String s = WiFi.SSID(i);
    const char *ssid = s.c_str();
    int32_t rssi = WiFi.RSSI(i);
    wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
    y = NULL;
    for (x = wifi_networks; x != NULL; x = x->next) {
      if (x->encryptionType == encryptionType && !strcmp(ssid, x->ssid)) {
        x->accessPoints++;
        if (x->bestRSSI < rssi) x->bestRSSI = rssi;
        break;
      }
      y = x;
    }
    if (!x) {
      struct sWifiNetwork *n = (struct sWifiNetwork *)malloc(sizeof(struct sWifiNetwork));
      if (y) y->next = n;
      else wifi_networks = n;
      n->accessPoints = 1;
      n->encryptionType = encryptionType;
      n->bestRSSI = rssi;
      n->next = NULL;
      strncpy(n->ssid, ssid, sizeof(n->ssid)); n->ssid[sizeof(n->ssid)-1] = 0;
    }
  }
  WiFi.scanDelete();
  Serial.println("Found these WLAN networks:");
  
  for (x = wifi_networks; x != NULL; x = x->next) {
    char buf[80];
    snprintf(buf, sizeof(buf), "  %-32s %3d %5d %s\n", x->ssid, x->accessPoints, x->bestRSSI, wifi_enctype2String(x->encryptionType));
    Serial.print(buf);
  }
}

wifi_mode_t wifi_mode;

void wifi_set_ap_mode() {
  long unsigned int espmac = ESP.getEfuseMac() >> 24;
  char ssid[20];
  char password[20] = "";
  snprintf(ssid, sizeof(ssid), "ESP32-%06lX", espmac);
  Serial.printf("Setting WLAN AP mode, SSID=\"%s\"\n", ssid);
  wifi_password[0] = 0;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, "");  // Set up the SoftAP
  MDNS.begin("esp32");
  wifi_ip = WiFi.softAPIP();
  Serial.printf("AP: %s, PASS: %s, IP: %s\n", ssid, password[0]?password:"(none)", wifi_ip.toString());
  wifi_loop_state = WIFI_LOOP_STATE_AP_MODE;
  if (handler_ap_mode) (*handler_ap_mode)();
  wifi_mode = WIFI_AP;
}

void wifi_set_sta_mode() {
  Serial.printf("Setting WLAN STA mode, hostname=\"%s\" SSID=\"%s\"\n", wifi_hostname, wifi_ssid);
  WiFi.mode(WIFI_MODE_NULL);
  WiFi.setHostname(wifi_hostname);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(wifi_ssid, wifi_password);
  wifi_initialization_time = millis();
  wifi_mode = WIFI_STA;
}

void wifi_restart() {
  if (wifi_mode == WIFI_AP) wifi_set_ap_mode();
  else if (wifi_mode == WIFI_STA) wifi_set_sta_mode();
}

void wifi_loop() {
  Preferences prefs;
  bool have_ssid = false;
  switch (wifi_loop_state) {
  case WIFI_LOOP_STATE_INIT:
    have_ssid = !!wifi_ssid[0];
    if (!have_ssid) {
      wifi_set_ap_mode();
    }
    else {
      wifi_set_sta_mode();
      WiFi.macAddress(mac);
      Serial.printf("WLAN MAC: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      wifi_loop_state = WIFI_LOOP_STATE_WAITING_FOR_CONNECTION;
    }
    break;
  case WIFI_LOOP_STATE_WAITING_FOR_CONNECTION:
    if (WiFi.status() == WL_CONNECTED) {
      wifi_loop_state = WIFI_LOOP_STATE_CONNECTION_ESTABLISHED;
    }
    else {
      unsigned long m = millis();
      if (m - wifi_initialization_time >= WIFI_CONNECTION_TIMEOUT) {
        Serial.println("WLAN connection not working, keep trying.");
        wifi_initialization_time = m;
      }
    }
    break;
  case WIFI_LOOP_STATE_CONNECTION_ESTABLISHED:
    Serial.print("WLAN connection established. IP address: ");
    wifi_ip = WiFi.localIP();
    Serial.println(wifi_ip);
    wifi_loop_state = WIFI_LOOP_STATE_CONNECTED;
    if (handler_connected) (*handler_connected)();
    break;
  case WIFI_LOOP_STATE_CONNECTED:
    if (WiFi.status() != WL_CONNECTED) {
      wifi_loop_state = WIFI_LOOP_STATE_INIT;
    }
    break;
  case WIFI_LOOP_STATE_AP_MODE:
    break;
  }
}



void wifi_show_config_form() {
  if (!webserver_admin_auth()) return;
  String reply = R"(<!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP Network Config</title>
  </head>
  <body style='width:480px'>
    <h2>ESP Network Config</h2>
    <form name='netconfig' method='POST' enctype='multipart/form-data' id='netconfig-form'>
      <table border=0>
        <tr><td align='right'>Hostname</td><td><input type='text' name='hostname' value='[@HOST]' size=32 /></td></tr>
        <tr><td align='right'>SSID</td><td><input type='text' name='ssid' value='[@SSID]' size=32 /></td></tr>
        <tr><td align='right'>Password</td><td><input type='text' name='password' value='[@PASS]' size=64 /></td></tr>
        <tr><td>&nbsp;</td><td><input type='submit' value='Save and restart'/></td><td>&nbsp;</td></tr>
      </table>
    </form>
    <br/><br/>
    <h3>Networks found in the neighborhood</h3>
    <table border=1>
     <tr><th>SSID</th><th>#APs</th><th>RSSI</th><th>Security</th><th>&nbsp;</th><tr>
  )";

  reply.replace("[@HOST]", webserver_quote_special(wifi_hostname));
  reply.replace("[@SSID]", webserver_quote_special(wifi_ssid));
  reply.replace("[@PASS]", webserver_quote_special(wifi_password));

  for (struct sWifiNetwork *x = wifi_networks; x != NULL; x = x->next) {
    String ssid = webserver_quote_special(x->ssid);
    String line;
    line.concat("<tr><td>");
    line.concat(ssid);
    line.concat("</td><td align=right>");
    line.concat(x->accessPoints);
    line.concat("</td><td align=right>");
    line.concat(x->bestRSSI);
    line.concat(" dBm</td><td>");
    line.concat(wifi_enctype2String(x->encryptionType));
    line.concat("</td><td><button onClick='document.getElementById(\"netconfig-form\").ssid.value=\"");
    line.concat(ssid);
    line.concat("\"; document.getElementById(\"netconfig-form\").password.value=\"\"'>Copy</button></td></tr>\n");
    reply.concat(line);
  }

  reply.concat(R"(
    </table>
  </body>
  )");
  webserver.send(200, "text/html", reply);
}

void wifi_save_config_form() {
  if (webserver.hasArg("hostname")) {
    String x = webserver.arg("hostname");
    const char *xc = x.c_str();
    strncpy(wifi_hostname, xc, sizeof(wifi_hostname)-1); wifi_hostname[sizeof(wifi_hostname)-1] = 0; 
  }
  if (webserver.hasArg("ssid")) {
    String x = webserver.arg("ssid");
    const char *xc = x.c_str();
    strncpy(wifi_ssid, xc, sizeof(wifi_ssid)-1); wifi_ssid[sizeof(wifi_ssid)-1] = 0; 
  }
  if (webserver.hasArg("password")) {
    String x = webserver.arg("password");
    const char *xc = x.c_str();
    strncpy(wifi_password, xc, sizeof(wifi_password)-1); wifi_ssid[sizeof(wifi_password)-1] = 0; 
  }
  wifi_writePreferences();
  
  webserver.sendHeader("Refresh", "10");
  webserver.sendHeader("Location", String("http://")+wifi_hostname+".local/");
  webserver.send(307);
  ESP.restart();
}



void wifi_init() {
  wifi_readPreferences();
  wifi_loop_state = WIFI_LOOP_STATE_INIT;
}

void wifi_setup() {
  wifi_scan();
  wifi_init();
  webserver.on(URI_NETCONFIG, HTTP_GET, wifi_show_config_form);
  webserver.on(URI_NETCONFIG, HTTP_POST, wifi_save_config_form);

  
}