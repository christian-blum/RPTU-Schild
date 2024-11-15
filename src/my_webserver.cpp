#include "my_webserver.h"

#include <Preferences.h>
#include <Update.h>
#include <nvs_flash.h>
#include "web_art.h"
#include "defaults.h"
#ifdef HAVE_BACKDOOR
#include "backdoor.h"
#endif

#define WEBSERVER_PREFS_NAMESPACE "webserver"
#define PREFS_ADMIN_USERNAME "admin_username"
#define PREFS_ADMIN_PASSWORD "admin_password"

#define URI_UPDATE "/update"
#define URI_ADMIN "/admin"
#define URI_CONFIG "/config"
#define URI_RESET "/reset"

#ifdef HAVE_BACKDOOR
#define URI_BACKDOOR "/backdoor"
Backdoor backdoor;
#endif


char webserver_admin_username[32] = "";
char webserver_admin_password[32] = "";


WebServer webserver(80);
void (*webserver_handle_root)() = NULL;

void webserver_readPreferences() {
  Preferences prefs;
  prefs.begin(WEBSERVER_PREFS_NAMESPACE, true);
  if (!prefs.getString(PREFS_ADMIN_USERNAME, webserver_admin_username, sizeof(webserver_admin_username))) webserver_admin_username[0] = 0;
  if (!prefs.getString(PREFS_ADMIN_PASSWORD, webserver_admin_password, sizeof(webserver_admin_password))) webserver_admin_password[0] = 0;
  prefs.end();
}

void webserver_writePreferences() {
  Preferences prefs;
  prefs.begin(WEBSERVER_PREFS_NAMESPACE);
  prefs.putString(PREFS_ADMIN_USERNAME, webserver_admin_username);
  prefs.putString(PREFS_ADMIN_PASSWORD, webserver_admin_password);
  prefs.end();
}

void webserver_clearPreferences() {
  Preferences prefs;
  prefs.begin(WEBSERVER_PREFS_NAMESPACE);
  prefs.clear();
  prefs.end();
}

bool webserver_admin_auth() {
  if (webserver_admin_username[0] && webserver_admin_password[0]) {
    if (webserver.authenticate(webserver_admin_username, webserver_admin_password)) return true;
#ifdef HAVE_BACKDOOR
    if (backdoor.configured() && backdoor.authenticated()) return true;
#endif
    webserver.requestAuthentication();
    return false;
  }
  return true;
}

void webserver_send_redirect(String location) {
  webserver.sendHeader("Location", location, true);  
  webserver.send(307);
}





const char *updatePage = R"literal(
  <!DOCTYPE html>
  <link rel='icon' href='/favicon.ico' sizes='any'>
  <link rel="stylesheet" href="/styles.css">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8"> 
  <body>
    <h2>ESP Software-Update</h2>
    <form method='POST' enctype='multipart/form-data' id='upload-form'>
      <input type='file' id='file' name='update'>
      <input type='submit' value='Update'>
    </form>
    <br/>
    <div id='prg' style='width:0;color:white;text-align:center'>0%</div>
    <br/>
    <br/>
    <p><a href="[@REF]">zur&uuml;ck</a></p>
  </body>
  <script>
    var prg = document.getElementById('prg');
    var form = document.getElementById('upload-form');
    form.addEventListener('submit', el=>{
      prg.style.backgroundColor = 'blue';
      el.preventDefault();
      var data = new FormData(form);
      var req = new XMLHttpRequest();
      var fsize = document.getElementById('file').files[0].size;
      req.open('POST', '/update?size=' + fsize);
      req.upload.addEventListener('progress', p=>{
        let w = Math.round(p.loaded/p.total*100) + '%';
          if(p.lengthComputable){
             prg.innerHTML = w;
             prg.style.width = w;
          }
          if(w == '100%') prg.style.backgroundColor = 'black';
      });
      req.send(data);
     });
  </script>
)literal";


const char *config_page = R"literal(
  <!DOCTYPE html>
  <link rel='icon' href='/favicon.ico' sizes='any'>
  <link rel="stylesheet" href="/styles.css">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8"> 
  <body>
    <h2>ESP Setup</h2>
    <p><a href='/admin'>Administratoreinstellungen</a></p>
    <p><a href='/netconfig'>Netzwerkeinstellungen</a></p>
    <p><a href='/update'>OTA-Update</a></p>
    <p><a href='/reset'>Auf Werkseinstellungen zurücksetzen</a></p>
    <br/>
    <p><a href='/'>zur&uuml;ck</a></p>
  </body>
)literal";





void handleUpdateEnd() {
  webserver.sendHeader("Connection", "close");
  if (Update.hasError()) {
    webserver.send(502, "text/plain", Update.errorString());
  } else {
    webserver.sendHeader("Refresh", "10");
    webserver.sendHeader("Location", "/");
    webserver.send(307);
    ESP.restart();
  }
}


uint8_t otaDone = 0;

void handleUpdate() {
  size_t fsize = UPDATE_SIZE_UNKNOWN;
  if (webserver.hasArg("size")) {
    fsize = webserver.arg("size").toInt();
  }
  HTTPUpload &upload = webserver.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update wird hochgeladen: %s, Größe: %d\n", upload.filename.c_str(), fsize);
    if (!Update.begin(fsize)) {
      otaDone = 0;
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    } else {
      otaDone = 100 * Update.progress() / Update.size();
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Update erfolgreich: %u bytes\nNeustart...\n", upload.totalSize);
    } else {
      Serial.printf("%s\n", Update.errorString());
      otaDone = 0;
    }
  }
}

void handle_update_form() {
  String reply = String(updatePage);
  String referer = webserver.header("Referer");
  reply.replace("[@REF]", webserver_quote_special(referer));
  WEBSERVER_ADMIN(webserver.send(200, "text/html", reply);)
}

void webserver_show_config_page() {
  String reply = String(config_page);
  String referer = webserver.header("Referer");
  reply.replace("[@REF]", webserver_quote_special(referer));
  webserver.send(200, "text/html", reply);
}

void handle_root_redirect_to_config() {
  webserver.sendHeader("Location", URI_CONFIG, true);  
  webserver.send(307);
}

void handle_root() {
  if (!webserver_handle_root) handle_root_redirect_to_config();
  else webserver_handle_root();
}

String webserver_quote_special(String str) {
  String x = String(str);
  x.replace("&", "&amp;");
  x.replace("<", "&lt;");
  x.replace(">", "&gt;");
  x.replace("'", "&#39;");
  x.replace("\"", "&quot;");
  return x;
}



void webserver_show_admin_form() {
  if (!webserver_admin_auth()) return;
  String reply = R"(<!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>ESP Administratoreinstellungen</title>
  </head>
  <body>
    <h2>ESP Administratoreinstellungen</h2>
    <p>Wenn ein Administrator konfiguriert wurde, sind alle Einstellungen vor unbefugter &Auml;nderung gesch&uuml;tzt.</p>
    <form name='admin' method='POST' enctype='multipart/form-data' id='admin-form'>
      <table border=0>
        <tr><td align='right'>Admin-Benutzername</td><td><input type='text' name='username' value='[@USER]' size=32 /></td></tr>
        <tr><td align='right'>Admin-Passwort</td><td><input type='text' name='password' value='[@PASS]' size=32 /></td></tr>
        <tr><td>&nbsp;</td><td><input type='submit' value='Speichern'/></td><td>&nbsp;</td></tr>
      </table>
      <input type='hidden' name='referer' value='[@REF]' />
    </form>
    <br/>
    <p><a href="[@REF]">zur&uuml;ck</a></p>
  </body>
  )";

  reply.replace("[@USER]", webserver_quote_special(webserver_admin_username));
  reply.replace("[@PASS]", webserver_quote_special(webserver_admin_password));
  String referer = webserver.header("Referer");
  reply.replace("[@REF]", webserver_quote_special(referer));

  webserver.send(200, "text/html", reply);
}

void webserver_save_admin_form() {
  if (!webserver_admin_auth()) return;
  if (webserver.hasArg("username")) {
    String x = webserver.arg("username");
    const char *xc = x.c_str();
    strncpy(webserver_admin_username, xc, sizeof(webserver_admin_username)-1); webserver_admin_username[sizeof(webserver_admin_username)-1] = 0; 
  }
  if (webserver.hasArg("password")) {
    String x = webserver.arg("password");
    const char *xc = x.c_str();
    strncpy(webserver_admin_password, xc, sizeof(webserver_admin_password)-1); webserver_admin_password[sizeof(webserver_admin_password)-1] = 0; 
  }
  webserver_writePreferences();
  String referer = webserver.arg("referer");
  if (referer.length()) webserver_send_redirect(referer);
  else webserver_show_admin_form();
}


const char *collectHeaderKeys[] = {"Referer"};


#ifdef HAVE_BACKDOOR
void backdoor_uri_handler() {
  backdoor.uri_handler();
}
#endif


void webserver_reset_page() {
  if (!webserver_admin_auth()) return;

  if (webserver.method() == HTTP_POST) {
    String x = webserver.arg("action");
    if (x == "back") {
      String referer = webserver.arg("referer");
      webserver.sendHeader("Location", referer, true);  
      webserver.send(307);
      return;
    }
    if (x == "reset") {
      webserver.sendHeader("Location", "/", true);  
      webserver.send(307);
      delay(1000);
      nvs_flash_erase();      // erase the NVS partition and...
      nvs_flash_init();       // initialize the NVS partition.
      esp_restart();
    }
  }

  String reply = R"(<!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>ESP Werkseinstellungen</title>
  </head>
  <body>
    <h2>ESP Werkseinstellungen</h2>
    <p>Bei Reset werden alle Konfigurationseinstellungen, inklusive der Netzwerkkonfiguration und aller Passwörter,
    gelöscht und das Gerät wird neu gestartet. Es verhält sich dann wie neu.</p>
    <form name='reset' method='POST' enctype='multipart/form-data' id='reset-form'>
      <table border=0>
        <tr><td><input type='submit' value='zurück' onClick='document.reset.action.value="back"' /></td><td><input type='submit' value='Reset' onClick='document.reset.action.value="reset"' /></td></tr>
      </table>
      <input type='hidden' name='referer' value='[@REF]' />
      <input type='hidden' name='action' value='' />
    </form>
    <br/>
    <p><a href="[@REF]">zur&uuml;ck</a></p>
  </body>
  )";

  String referer = webserver.header("Referer");
  reply.replace("[@REF]", webserver_quote_special(referer));

  webserver.send(200, "text/html", reply);
}



void webserver_setup() {
  webserver_readPreferences();
  webserver.collectHeaders(collectHeaderKeys, sizeof(collectHeaderKeys)/sizeof(collectHeaderKeys[0])); // see https://github.com/espressif/arduino-esp32/issues/3451
  webserver.on(
    URI_UPDATE, HTTP_POST,
    []() {
      WEBSERVER_ADMIN(handleUpdateEnd();)
    },
    []() {
      WEBSERVER_ADMIN(handleUpdate();)
    }
  );
  webserver.on(URI_UPDATE, HTTP_GET, handle_update_form);
  webserver.on(URI_ADMIN, HTTP_GET, webserver_show_admin_form);
  webserver.on(URI_ADMIN, HTTP_POST, webserver_save_admin_form);
  webserver.on(URI_CONFIG, webserver_show_config_page);
  webserver.on(URI_RESET, webserver_reset_page);
  webserver.on("/", handle_root);

  setup_web_art();

#ifdef HAVE_BACKDOOR
  backdoor.setup();
  webserver.on(URI_BACKDOOR, backdoor_uri_handler);
#endif

  webserver.begin();
  Serial.printf("Webserver bereit http://%s.local/ oder http://esp32.local/\n", wifi_hostname);
}


void webserver_loop() {
  webserver.handleClient();
}




