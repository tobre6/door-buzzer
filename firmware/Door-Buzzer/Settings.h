#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

class Settings {
public:
  boolean load();
  void save();

  void setWifiSSID(String);
  String getWifiSSID();

  void setWifiPassword(String);
  String getWifiPassword();

  void setMQTTServer(String);
  String getMQTTServer();

private:

  struct CONTAINER {
    char wifiSSID[32];
    char wifiPassword[32];
    char mqttServer[32];
  } container;
};

#endif




