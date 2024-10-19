#include <Arduino.h>
#include <WebServer.h>
#include "config_pages.h"
#include "my_webserver.h"
#include "einstellungen.h"



const char *html_landing_page = R"literal(
  <!DOCTYPE html>
  <head>
    <meta charset="UTF-8"> 
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <title>RPTU Schild</title>
  </head>
  <body>
    <h1>RPTU Schild</h2>
    <p><a href='/config'>Allgemeine Konfiguration</a></p>
    <p><a href='/einstellungen'>Allgemeine Einstellungen</a></p>
    <p><a href='/uebergaenge'>Übergänge</a></p>
    <p><a href='/effekte'>Effekte</a></p>
    <br/>
    <hr/>
    <p><small>###CREDITS###</br/>###RELEASEINFO###</small></p>
  </body>
)literal";


#define URI_EINSTELLUNGEN "/einstellungen"

extern const char *credits;
extern const char *releaseInfo;

void config_pages_show_landing_page() {
  String s = String(html_landing_page);
  s.replace("###CREDITS###", webserver_quote_special(String(credits)));
  s.replace("###RELEASEINFO###", webserver_quote_special(String(releaseInfo)));
  webserver.send(200, "text/html", s);
}


const char *html_einstellungen = R"literal(
  <!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta charset="UTF-8"> 
    <title>RPTU-Schild - Einstellungen</title>
    <script>
      function resetValues() {
        document.getElementById('hell').value=###HELLIGKEIT###;
      }
    </script>
  </head>
  <body>
    <h1>RPTU Schild</h1>
    <h2>Allgemeine Einstellungen</h2>
    <form method='POST' name='form'>
      <table border=0 width='100%'>
        <tr><td align='left'>Anzeige Ein/Aus</td><td align='left'>Effekte Ein/Aus</td><td align='left'>Hintergrund schwarz</td></tr>
        <tr><td width="33%"><label class="switch"><input type="checkbox" name="einaus" ###EINAUS###><span class="slider_switch round"></span></label></td>
            <td width="33%"><label class="switch"><input type="checkbox" name="effekte" ###EFFEKTE###><span class="slider_switch round"></span></label></td>
            <td width="33%"><label class="switch"><input type="checkbox" name="hintergrund" ###HINTERGRUND###><span class="slider_switch round"></span></label></td></tr>
        <tr><td colspan="3" align='left'>Helligkeit</td></tr>
        <tr><td colspan="3" width='100%'><input type="range" min="2" max="255" name="hell" value="###HELLIGKEIT###" class="slider" id="hell"></td></tr>
      </table>
      <input type='button' value='Zurücksetzen' onClick='resetValues()'>&nbsp;<input type='button' value='Ändern' onClick='document.form.submit()'>
    </form>
    <br/>
    <hr/>
    <p><small>###CREDITS###</br/>###RELEASEINFO###</small></p>
  </body>
)literal";


#define DEBUG_POST

void config_pages_einstellungen() {
  HTTPMethod method = webserver.method();
  String s;
  bool e;
  uint8_t h;
  switch (method) {
  case HTTP_GET:
    s = String(html_einstellungen);
    s.replace("###HELLIGKEIT###", String(helligkeit));
    s.replace("###CREDITS###", webserver_quote_special(String(credits)));
    s.replace("###RELEASEINFO###", webserver_quote_special(String(releaseInfo)));
    s.replace("###EINAUS###", einaus ? "checked":"");
    s.replace("###EFFEKTE###", effekte_einaus ? "checked":"");
    s.replace("###HINTERGRUND###", hintergrund_schwarz ? "checked":"");
    webserver.send(200, "text/html", s);
    break;
  case HTTP_POST:
#ifdef DEBUG_POST
    int n = webserver.args();
    for (int i = 0; i < n; i++) {
      Serial.print(webserver.argName(i)); Serial.print("="); Serial.println(webserver.arg(i));
    }
#endif
    s = webserver.arg("hell");
    h = s.toInt();
    if (h != helligkeit) {
      helligkeit = h;
      preferences_speichern = true;
    }
    s = webserver.arg("einaus");
    e = (s == "on");
    if (e != einaus) {
      einaus = e;
      preferences_speichern = true;
    }
    s = webserver.arg("effekte");
    e = (s == "on");
    if (e != effekte_einaus) {
      effekte_einaus = e;
      preferences_speichern = true;
    }
    s = webserver.arg("hintergrund");
    e = (s == "on");
    if (e != hintergrund_schwarz) {
      hintergrund_schwarz = e;
      preferences_speichern = true;
    }
    webserver_send_redirect("/");
    break;
  }
}



void setup_config_pages() {
  webserver_handle_root = config_pages_show_landing_page;
  webserver.on(URI_EINSTELLUNGEN, config_pages_einstellungen);
}