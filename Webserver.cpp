#include <ESP8266WebServer.h>

#include "Webserver.h"

const char compile_date[] = __DATE__ " " __TIME__;

Webserver::Webserver() {
  server = new ESP8266WebServer(80);
  Serial.println(compile_date);

  server->on("/", [&](){

    String html = "<h1>Door Buzzer</h1>" + String("<p>Built: ") + String(compile_date) + "</p>";
    
    this->server->send(200, "text/html", html);
  });

  server->begin();
}

void Webserver::loop() {
  server->handleClient();
}

