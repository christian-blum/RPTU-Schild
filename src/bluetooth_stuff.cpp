#include "bluetooth_stuff.h"
#ifdef HAVE_BLUETOOTH

#include <Arduino.h>
#include <BluetoothSerial.h>

#include "osd.h"
#include "gimp_artwork.h"
#include "defaults.h"
#include "einstellungen.h"

// NEIN, das wird vermutlich nicht auf Anhieb funktionieren

#undef HAVE_DELETEALLBONDEDDEVICES

BluetoothSerial SerialBT;

uint8_t bt_einaus;
String bt_device_name;
uint8_t osd_bt_level;


void BT_neu_pairen() {
#ifdef HAVE_DELETEALLBONDEDDEVICES
  SerialBT.deleteAllBondedDevices();
#endif
}





#define BT_BUFFER_SIZE 128
char bt_buffer[BT_BUFFER_SIZE];
int bt_buffer_inptr;


//typedef enum {
//    ESP_SPP_INIT_EVT                    = 0,                /*!< When SPP is initialized, the event comes */
//    ESP_SPP_UNINIT_EVT                  = 1,                /*!< When SPP is deinitialized, the event comes */
//    ESP_SPP_DISCOVERY_COMP_EVT          = 8,                /*!< When SDP discovery complete, the event comes */
//    ESP_SPP_OPEN_EVT                    = 26,               /*!< When SPP Client connection open, the event comes */
//    ESP_SPP_CLOSE_EVT                   = 27,               /*!< When SPP connection closed, the event comes */
//    ESP_SPP_START_EVT                   = 28,               /*!< When SPP server started, the event comes */
//    ESP_SPP_CL_INIT_EVT                 = 29,               /*!< When SPP client initiated a connection, the event comes */
//    ESP_SPP_DATA_IND_EVT                = 30,               /*!< When SPP connection received data, the event comes, only for ESP_SPP_MODE_CB */
//    ESP_SPP_CONG_EVT                    = 31,               /*!< When SPP connection congestion status changed, the event comes, only for ESP_SPP_MODE_CB */
//    ESP_SPP_WRITE_EVT                   = 33,               /*!< When SPP write operation completes, the event comes, only for ESP_SPP_MODE_CB */
//    ESP_SPP_SRV_OPEN_EVT                = 34,               /*!< When SPP Server connection open, the event comes */
//    ESP_SPP_SRV_STOP_EVT                = 35,               /*!< When SPP server stopped, the event comes */
//    ESP_SPP_VFS_REGISTER_EVT            = 36,               /*!< When SPP VFS register, the event comes */
//    ESP_SPP_VFS_UNREGISTER_EVT          = 37,               /*!< When SPP VFS unregister, the event comes */
//} esp_spp_cb_event_t;



void bt_reply(const char *reply) {
  SerialBT.write((const uint8_t *)reply, strlen(reply));
  SerialBT.write('\r');
  SerialBT.write('\n');
  SerialBT.flush();
}

#define BT_OK_TAG "OK"
void bt_reply_ok() {
  bt_reply(BT_OK_TAG);
}

#define BT_ERR_TAG "ERR "
void bt_reply_err(const char *desc) {
  SerialBT.write((const uint8_t *)BT_ERR_TAG, sizeof(BT_ERR_TAG));
  SerialBT.write((const uint8_t *)desc, strlen(desc));
  SerialBT.write('\r');
  SerialBT.write('\n');
  SerialBT.flush();
}


#define SEP " \t"

void bt_command(char *cmd) {
#ifdef DEBUG
  Serial.print("> ");
  Serial.println(cmd);
#endif
  char *command = strtok(cmd, SEP);
  if (!strcmp("set", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      char *arg2 = strtok(NULL, SEP);
      int x = strtol(arg2, NULL, 10);
      if (x >= 0 || x <= 255) { 
        helligkeit = x;
        preferences_speichern = true;
        bt_reply_ok();
      }
      else {
        bt_reply_err("Wert unzulässig");
      }
    }
    else if (strcmp(PREF_BT_DEVICE_NAME, arg1)) {
      char *arg2 = strtok(NULL, NULL);
      bt_device_name = arg2;
      bt_reply_ok();
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else if (!strcmp("get", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      char num[4];
      sprintf(num, "%d", helligkeit);
      bt_reply(num);
    }
    else if (strcmp(PREF_BT_DEVICE_NAME, arg1)) {
      char buf[32];
      bt_device_name.toCharArray(buf, sizeof(buf)-1);
      buf[sizeof(buf)-1] = 0;
      bt_reply(buf);
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else if (!strcmp("inc", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      if (helligkeit < 255) {
        helligkeit++;
        preferences_speichern = true;
      }
      char num[4];
      sprintf(num, "%d", helligkeit);
      bt_reply(num);
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else if (!strcmp("dec", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      if (helligkeit > 0) {
        helligkeit--;
        preferences_speichern = true;
      }
      char num[4];
      sprintf(num, "%d", helligkeit);
      bt_reply(num);
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else {
    bt_reply_err("Unbekanner Befehl");
  }
}

void bt_clear() {
  Serial.println("*** BT connect ***");
}


void bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) { // Verbindungsaufbau durch die Client-App
    bt_clear();
  }
  else if (event == ESP_SPP_DATA_IND_EVT) { // Client-App hat Daten geschickt
    while (SerialBT.available()) {
      char c = SerialBT.read();
      if (c == '\n') {}
      else if (c == '\r') {
        bt_buffer[bt_buffer_inptr] = '\0';
        bt_command(bt_buffer);
        bt_buffer_inptr = 0;
      }
      else if (bt_buffer_inptr < BT_BUFFER_SIZE - 1) bt_buffer[bt_buffer_inptr++] = c;
    }
  }
  else {
    Serial.print("??? Unknown callback event "); Serial.print(event); Serial.println();
  }
}

void bt_setup() {
  // Wir sparen uns den Authentisierungs-Schnickschnack und machen das auf der höheren Ebene.
  // Wie man SSP macht, findest du hier: https://github.com/espressif/arduino-esp32/tree/master/libraries/BluetoothSerial/examples
  if (bt_einaus) SerialBT.begin(bt_device_name);
  if (bt_einaus == 2) {
#ifdef HAVE_DELETEALLBONDEDDEVICES
    SerialBT.deleteAllBondedDevices();
#endif
    bt_einaus = 0;
    preferences_speichern = true;
  }
  SerialBT.register_callback(bt_callback);
}



void bt_loop() {

}

#endif
