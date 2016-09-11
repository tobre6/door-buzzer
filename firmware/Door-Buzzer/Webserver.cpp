#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>

#include "Webserver.h"

const char compile_date[] = __DATE__ " " __TIME__;

Webserver::Webserver() {
  server = new ESP8266WebServer(80);
  Serial.println(compile_date);

  server->on("/", [&](){

    String html = "<html><head><title>Door Buzzer</title>";
    html += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" crossorigin=\"anonymous\">";
    html += "<link href=\"https://maxcdn.bootstrapcdn.com/font-awesome/4.6.3/css/font-awesome.min.css\" rel=\"stylesheet\" />";
    html += "</head><body>";
    html += "<div class=\"container\"><div class=\"starter-template\">";
    html += "<h1>Door Buzzer</h1>";
    html += "<p class=\"lead\">Version: XXX<br/>";
    html += "Built: " + String(compile_date) + "<br/>";
    html += "Uptime: ZZZ</p>";
    html += "<p><a href=\"ota\" class=\"btn btn-primary\"><i class=\"fa fa-refresh\" aria-hidden=\"true\"></i> Upgrade</a></p>";
    html += "</div></div></body></html>";
    
    this->server->send(200, "text/html", html);
  });

  server->on("/ota", [&](){
    Serial.println("Commence OTA");
    this->server->sendHeader("Location", "/", true);
    this->server->send ( 302, "text/plain", "");
    ESPhttpUpdate.update("ota.clockwise.ee", 80, "/Door-Buzzer.bin");
  });

  server->begin();
}

void Webserver::loop() {
  server->handleClient();
}

