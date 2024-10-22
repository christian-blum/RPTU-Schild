#include <Arduino.h>
#include <WebServer.h>
#include "config_pages.h"
#include "my_webserver.h"
#include "einstellungen.h"
#include "uebergaenge.h"


const char *html_landing_page = R"literal(
  <!DOCTYPE html>
  <head>
    <meta charset="UTF-8"> 
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>RPTU-Schild</title>
  </head>
  <body>
    <h1>RPTU-Schild</h2>
    <p><a href='/config'>Setup</a></p>
    <p><a href='/einstellungen'>Allgemeine Einstellungen</a></p>
    <p><a href='/uebergaenge'>Übergänge</a></p>
    <p><a href='/effekte'>Effekte</a></p>
    <br/>
    <hr/>
    <p><small>###CREDITS###<br/>###RELEASEINFO###</small></p>
  </body>
)literal";


#define URI_EINSTELLUNGEN "/einstellungen"
#define URI_UEBERGAENGE "/uebergaenge"

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
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8"> 
    <title>RPTU-Schild - Einstellungen</title>
    <script>
      function resetValues() {
        document.getElementById('hell').value=###HELLIGKEIT###;
      }
    </script>
  </head>
  <body>
    <h1>RPTU-Schild</h1>
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
    <p><small>###CREDITS###<br/>###RELEASEINFO###</small></p>
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
    s = html_einstellungen;
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

const char *html_uebergaenge_anfang = R"literal(
  <!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8"> 
    <title>RPTU-Schild - Übergänge</title>
  </head>
  <body>
    <h1>RPTU-Schild</h1>
    <h2>Übergänge</h2>
    <form method='POST' name='form'>
      <table border=0 width='100%'>
)literal";
const char *html_uebergaenge_ende = R"literal(
      </table>
      <input type='button' value='Zurück zum Menü' onClick='document.getElementById("back").value="back"; document.form.submit()'>&nbsp;<input type='button' value='Ändern' onClick='document.form.submit()'>
      <input type='hidden' name='defaults_laden' id='defaults_laden' value=''>
      <input type='hidden' name='back' id='back' value=''>
    </form>
    <br/>
    <hr/>
    <p><small>###CREDITS###<br/>###RELEASEINFO###</small></p>
  </body>
)literal";


void config_pages_uebergang(String &s, Uebergang *u) {
  s += "<tr><td valign='top'><b>";
  s += webserver_quote_special(u->name);
  s += "</b><br/><small>";
  s += webserver_quote_special(u->beschreibung);
  s += "</small></td><td align='left' valign='top'>";
  s += "<table border=0 width='100%'>";
  for (int i = 0; i < u->parameter.size(); i++) {
    struct sUebergangParameter *p = &u->parameter[i];
    s += "<tr><td align='right'>";
    s += webserver_quote_special(p->name);
    s += ":</td><td><span style='white-space: nowrap;'>";
    String _tag = u->tag;
    _tag += "$";
    _tag += p->tag;
    switch (p->typ) {
      case UPT_BOOL:
        s += "<input type='hidden' name='";
        s += _tag;
        s += "' id='";
        s += _tag;
        s += "' value='";
        s += (*((bool *)p->variable)) ? "on" : "off";
        s += "'><input type='checkbox' onchange='document.getElementById(\"";
        s += _tag;
        s += "\").value = this.checked ? \"on\" : \"off\"'";
        if (*((bool *)p->variable)) s += " checked";
        s += ">";
        break;
      case UPT_USHORT:
      case UPT_FLOAT:
        s += "<input type='text' size='";
        s += p->laenge;
        s += "' name='";
        s += _tag;
        s += "' value='";
        if (p->typ == UPT_USHORT) s += *((uint16_t *)p->variable);
        else if (p->typ == UPT_FLOAT) s += *((float *)p->variable);
        s += "'>";
        if (p->einheit) { s += "&nbsp;"; s += webserver_quote_special(p->einheit); } 
        break;
    }
    s += "</span></td></tr>";
  }
  s += "</table></td><td  valign='top'><input type='button' onclick='document.getElementById(\"defaults_laden\").value=\"";
  s += u->tag;
  s += "\"; document.form.submit()' value='Defaults'></td></tr>\n";
}

void config_pages_uebergaenge() {
  HTTPMethod method = webserver.method();
  String s;
  String x;
  bool e;
  uint8_t h;
  bool back = false;
  switch (method) {
  case HTTP_POST:
    if ((x = webserver.arg("back")) == "back") {
      back = true;
    }
    else if ((x = webserver.arg("defaults_laden")) != "") {
      for (int i = 0; i < uebergaenge.size(); i++) {
        Uebergang *u = uebergaenge[i];
        if (x == u->tag) u->prefs_defaults();
      }
    }
    else {
      // jaja, das ist eklig ineffizient - aber es geht und es tut's
      for (int i = 0; i < webserver.args(); i++) {
        x = webserver.argName(i);
        int p = x.indexOf('$');
        if (p >= 0) {
          String utag = x.substring(0, p);
          String ptag = x.substring(p+1);
          for (int j = 0; j < uebergaenge.size(); j++) {
            Uebergang *u = uebergaenge[j];
            if (utag == u->tag) {
              for (int k = 0; k < u->parameter.size(); k++) {
                struct sUebergangParameter *p = &u->parameter[k];
                if (ptag == p->tag) {
                  switch(p->typ) {
                    case UPT_BOOL:
                      *((bool *)p->variable) = (webserver.arg(i) == "on");
                      break;
                    case UPT_USHORT:
                      *((uint16_t *)p->variable) = webserver.arg(i).toInt();
                      break;
                    case UPT_FLOAT:
                      *((float *)p->variable) = webserver.arg(i).toFloat();
                      break;
                  }
                }
              }
              break;
            }
          }
        }
      }
    }
    uebergaenge_prefs_schreiben();
    // fall through
  case HTTP_GET:
    s = html_uebergaenge_anfang;
    for (int i = 0; i < uebergaenge.size(); i++) {
      config_pages_uebergang(s, uebergaenge[i]);
    }
    s += html_uebergaenge_ende;
    s.replace("###CREDITS###", webserver_quote_special(String(credits)));
    s.replace("###RELEASEINFO###", webserver_quote_special(String(releaseInfo)));
    if (back) {
      webserver.sendHeader("Location", "/", true);  
      webserver.send(307);
    }
    else {
      webserver.send(200, "text/html", s);
    }
    break;
  }
}

void setup_config_pages() {
  webserver_handle_root = config_pages_show_landing_page;
  webserver.on(URI_EINSTELLUNGEN, config_pages_einstellungen);
  webserver.on(URI_UEBERGAENGE, config_pages_uebergaenge);
}