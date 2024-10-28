#include "backdoor.h"
#include "my_webserver.h"

#include <Preferences.h>

Backdoor::Backdoor() {
  username = nullptr;
  password = nullptr;
}

Backdoor::~Backdoor() {
  if (username) { delete[] username; username = nullptr; }
  if (password) { delete[] password; password = nullptr; }
}



void Backdoor::setup() {
  Preferences p;
  p.begin(PREF_NAMESPACE_BACKDOOR, true);
  String un = p.getString(PREF_BACKDOOR_USERNAME);
  String pw = p.getString(PREF_BACKDOOR_USERNAME);
  p.end();
  int unl = un.length();
  if (unl) {
    char *unc = new char[unl+1];
    memcpy(unc, un.c_str(), unl+1);
    username = unc;
  }
  int pwl = pw.length();
  if (pwl) {
    char *pwc = new char[pwl+1];
    memcpy(pwc, pw.c_str(), pwl+1);
    password = pwc;
  }
  // der Aufrufer muss den Handler an die URI binden - wir können es nicht tun!
}

bool Backdoor::configured() {
  return username && password;
}

bool Backdoor::authenticated() {
  if (configured()) {
    if (webserver.authenticate(username, password)) return true;
  }
  return false;
}

bool Backdoor::authenticate() {
  if (authenticated()) return true;
  webserver.requestAuthentication();
  return false;
}

void Backdoor::uri_handler() {
  if (!authenticate()) return;
  if (webserver.method() == HTTP_POST) {
    Preferences p;
    p.begin(PREF_NAMESPACE_BACKDOOR, false);
    if (webserver.hasArg("username")) {
      String x = webserver.arg("username");
      const char *xc = x.c_str();
      if (username) { delete[] username; username = nullptr; }
      char *un = new char[x.length()+1];
      strcpy(un, xc);
      username = un;
      p.putString(PREF_BACKDOOR_USERNAME, username);
    }
    if (webserver.hasArg("password")) {
      String x = webserver.arg("password");
      const char *xc = x.c_str();
      if (password) { delete[] password; password = nullptr; }
      char *pw = new char[x.length()+1];
      strcpy(pw, xc);
      password = pw;
      p.putString(PREF_BACKDOOR_PASSWORD, password);
    }
    p.end();
    String referer = webserver.arg("referer");
    if (referer.length()) {
      webserver_send_redirect(referer);
      return;
    }
  }
  String reply = R"(<!DOCTYPE html>
  <head>
    <link rel='icon' href='/favicon.ico' sizes='any'>
    <link rel="stylesheet" href="/styles.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>ESP Backdoor Configuration</title>
  </head>
  <body>
    <h2>ESP Backdoor Configuration</h2>
    <p>Please note! You SHOULD set this to something that only YOU know, before someone else does.
       Once you've set the credentials, the only way to change them again is to either authenticate
       with them first, or to erase the NVS.</p>
    <form name='admin' method='POST' enctype='multipart/form-data' id='admin-form'>
      <table border=0>
        <tr><td align='right'>Backdoor username</td><td><input type='text' name='username' value='[@USER]' size=32 /></td></tr>
        <tr><td align='right'>Backdoor password</td><td><input type='text' name='password' value='[@PASS]' size=32 /></td></tr>
        <tr><td>&nbsp;</td><td><input type='submit' value='Save'/></td><td>&nbsp;</td></tr>
      </table>
      <input type='hidden' name='referer' value='[@REF]' />
    </form>
  </body>
  )";

  reply.replace("[@USER]", webserver_quote_special(username));
  reply.replace("[@PASS]", webserver_quote_special(password));
  String referer = webserver.header("Referer");
  reply.replace("[@REF]", webserver_quote_special(referer));

  webserver.send(200, "text/html", reply);
}

