#include <Arduino.h>
#include <WebServer.h>
#include "config_pages.h"
#include "my_webserver.h"
#include "einstellungen.h"
#include "uebergaenge.h"
#include "effekte.h"
#include "effekt_laufschrift.h"


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
#define URI_EFFEKTE "/effekte"

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
        <tr><td align="right">Effekt Pause min.</td><td><input type="text" name="epmin" value="###EPMIN###">ms</td><td></td></tr>
        <tr><td align="right">Effekt Pause max.</td><td><input type="text" name="epmax" value="###EPMAX###">ms</td><td></td></tr>
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
  if (!webserver_admin_auth()) return;
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
    s.replace("###EPMIN###", String(effekt_pause_min));
    s.replace("###EPMAX###", String(effekt_pause_max));
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
    s = webserver.arg("epmin");
    uint32_t epmin = s.toInt();
    s = webserver.arg("epmax");
    uint32_t epmax = s.toInt();
    if (epmax < epmin) epmax = epmin;
    if (effekt_pause_min != epmin) {
      effekt_pause_min = epmin;
      preferences_speichern = true;
    }
    if (effekt_pause_max != epmax) {
      effekt_pause_max = epmax;
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
  if (!webserver_admin_auth()) return;
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
    if (back) {
      webserver.sendHeader("Location", "/", true);  
      webserver.send(307);
    }
    else {
      s = html_uebergaenge_anfang;
      for (int i = 0; i < uebergaenge.size(); i++) {
        config_pages_uebergang(s, uebergaenge[i]);
      }
      s += html_uebergaenge_ende;
      s.replace("###CREDITS###", webserver_quote_special(String(credits)));
      s.replace("###RELEASEINFO###", webserver_quote_special(String(releaseInfo)));
      webserver.send(200, "text/html", s);
    }
    break;
  }
}

void config_pages_effekt(String &s, Effekt *e) {
  String l;
  s += "<tr><td valign='top'><b>";
  s += webserver_quote_special(e->name);
  s += "</b><br/><small>";
  s += webserver_quote_special(e->beschreibung);
  s += "</small><br/>###LANG###</td><td align='left' valign='top'>";
  s += "<table border=0 width='100%'>";
  for (int i = 0; i < e->parameter.size(); i++) {
    struct sEffektParameter *p = &e->parameter[i];
    if (p->typ != EPT_TEXT) {
      s += "<tr><td align='right'>";
      s += webserver_quote_special(p->name);
      s += ":</td><td><span style='white-space: nowrap;'>";
    }
    String _tag = e->tag;
    _tag += "$";
    _tag += p->tag;
    switch (p->typ) {
      case EPT_TEXT:
        l += "<br/><span style='white-space: nowrap;'>";
        l += webserver_quote_special(p->name);
        l += ": ";
        if (e->loeschbar) {
          l += "<input type=text style='width: 95%;' name='";;
          l += _tag;
          l += "' value='";
          l += webserver_quote_special(*(char **)p->variable);
          l += "'>";
        }
        else {
          l += webserver_quote_special(*(char **)p->variable); // nicht veränderbar
        }
        l += "</span>";
        break;
      case EPT_BOOL:
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
      case EPT_USHORT:
      case EPT_SHORT:
      case EPT_FLOAT:
      case EPT_RGBA:
        s += "<input type='text' size='";
        s += p->laenge;
        s += "' name='";
        s += _tag;
        s += "' value='";
        if (p->typ == EPT_USHORT) s += *((uint16_t *)p->variable);
        else if (p->typ == EPT_SHORT) s += *((int16_t *)p->variable);
        else if (p->typ == EPT_FLOAT) s += *((float *)p->variable);
        else if (p->typ == EPT_RGBA) { char buf[9]; sprintf(buf,"%8.8x", ((struct sCRGBA *)p->variable)->x); s += buf; }
        s += "'>";
        if (p->einheit) { s += "&nbsp;"; s += webserver_quote_special(p->einheit); } 
        break;
    }
    s += "</span></td></tr>";
  }
  s += "</table></td><td  valign='top'><input type='button' onclick='document.getElementById(\"defaults_laden\").value=\"";
  s += e->tag;
  s += "\"; document.form.submit()' value='Defaults'></td></tr>\n";
  s.replace("###LANG###", l);
}

const char *html_effekte_anfang = R"literal(
  <!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="ISO-8859-1"> 
    <title>RPTU-Schild - Effekte</title>
  </head>
  <body>
    <h1>RPTU-Schild</h1>
    <h2>Effekte</h2>
    <form method='POST' name='form' accept-charset='ISO-8859-1'>
      <table border=0 width='100%'>
)literal";
const char *html_effekte_ende = R"literal(
      </table>
      <input type='button' value='Zur&uuml;ck zum Men&uuml;' onClick='document.getElementById("back").value="back"; document.form.submit()'>&nbsp;<input type='button' value='&Auml;ndern' onClick='document.form.submit()'>
      <input type='hidden' name='defaults_laden' id='defaults_laden' value=''>
      <input type='hidden' name='back' id='back' value=''>
    </form>
    <br/>
    <hr/>
    <p><small>###CREDITS###<br/>###RELEASEINFO###</small></p>
  </body>
)literal";

void config_pages_effekte() {
  if (!webserver_admin_auth()) return;
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
      for (int i = 0; i < effekte.size(); i++) {
        Effekt *e = effekte[i];
        if (x == e->tag) e->prefs_defaults();
      }
    }
    else {
      // jaja, das ist eklig ineffizient - aber es geht und es tut's
      for (int i = 0; i < webserver.args(); i++) {
        x = webserver.argName(i);
        int p = x.indexOf('$');
        if (p >= 0) {
          String etag = x.substring(0, p);
          String ptag = x.substring(p+1);
          for (int j = 0; j < effekte.size(); j++) {
            Effekt *e = effekte[j];
            if (etag == e->tag) {
              for (int k = 0; k < e->parameter.size(); k++) {
                struct sEffektParameter *p = &e->parameter[k];
                if (ptag == p->tag) {
                  switch(p->typ) {
                    case EPT_BOOL:
                      *((bool *)p->variable) = (webserver.arg(i) == "on");
                      break;
                    case EPT_SHORT:
                      *((int16_t *)p->variable) = webserver.arg(i).toInt();
                      break;
                    case EPT_USHORT:
                      *((uint16_t *)p->variable) = webserver.arg(i).toInt();
                      break;
                    case EPT_FLOAT:
                      *((float *)p->variable) = webserver.arg(i).toFloat();
                      break;
                    case EPT_TEXT:
                      if (static_cast<Effekt_Laufschrift*>(e) != nullptr) {
                        if (!strcmp("text", p->tag)) {
                          ((Effekt_Laufschrift *)e)->neuer_text(webserver.arg(i).c_str());
                        }
                      }
                      break;
                    case EPT_RGBA:
                      ((struct sCRGBA *)p->variable)->x = strtol(webserver.arg(i).c_str(), NULL, 16);
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
    effekte_prefs_schreiben();
    // fall through
  case HTTP_GET:
    if (back) {
      webserver.sendHeader("Location", "/", true);  
      webserver.send(307);
    }
    else {
      s = html_effekte_anfang;
      for (int i = 0; i < effekte.size(); i++) {
        config_pages_effekt(s, effekte[i]);
      }
      s += html_effekte_ende;
      s.replace("###CREDITS###", webserver_quote_special(String(credits)));
      s.replace("###RELEASEINFO###", webserver_quote_special(String(releaseInfo)));
      webserver.send(200, "text/html", s);
    }
    break;
  }
}











void setup_config_pages() {
  webserver_handle_root = config_pages_show_landing_page;
  webserver.on(URI_EINSTELLUNGEN, config_pages_einstellungen);
  webserver.on(URI_UEBERGAENGE, config_pages_uebergaenge);
  webserver.on(URI_EFFEKTE, config_pages_effekte);
}