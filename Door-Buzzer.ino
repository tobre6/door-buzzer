#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "default_settings.h"
#include "Webserver.h"
#include "Settings.h"

#define MQTT_CLIENT "Door-Buzzer2"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "home/doorbuzzer2"

#define RELAY 4
#define BUZZER_ON_TIME 20 // In seconds
#define CONNECTION_CHECK_INTERVAL 10 // In seconds

extern "C" { 
  #include "user_interface.h" 
}

WiFiClient wifiClient;
PubSubClient *mqttClient;
Webserver *webserver;
Settings *settings;
unsigned long timeBuzzerTurnedOn = 0;
unsigned long lastConnectionCheckTime = 0;

bool connectToWifi();
void connectToMqtt();
void mqttCallback(const MQTT::Publish&);
void turnOffBuzzer();
void turnOnBuzzer();
void checkConnection();

void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);

  webserver = new Webserver;
  settings = new Settings;
  if (!settings->load()) {
    Serial.println("Initializing settings");
    #ifdef DEFAULT_SETTINGS
      settings->setWifiSSID(WIFI_SSID);
      settings->setWifiPassword(WIFI_PASS);
      settings->setMQTTServer(MQTT_SERVER);
    #endif
    settings->save();
  }

  mqttClient = new PubSubClient(wifiClient, settings->getMQTTServer(), MQTT_PORT);
  mqttClient->set_callback(mqttCallback);
  if (connectToWifi()) {
    connectToMqtt();
  }
}

void loop() {
  mqttClient->loop();
  webserver->loop();

  if (timeBuzzerTurnedOn > 0 && millis() > timeBuzzerTurnedOn + 1000 * BUZZER_ON_TIME) {
    turnOffBuzzer();
  }

  if (millis () > lastConnectionCheckTime + 1000 * CONNECTION_CHECK_INTERVAL) {
    checkConnection();
  }
}

bool connectToWifi() {
  Serial.println("Connecting to Wifi");
  int retries = 10;
  WiFi.begin(settings->getWifiSSID().c_str(), settings->getWifiPassword().c_str());
  while ((WiFi.status() != WL_CONNECTED) && retries--) {
    delay(500);
    Serial.print(" .");
  }

  if (WiFi.status() == WL_CONNECTED) { 
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("Failed to connect to Wifi");
  return false;
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT");
  int retries = 10;

  while (!mqttClient->connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(90)) && retries--) {
    Serial.print(" .");
    delay(1000);
  }

  if(mqttClient->connected()) {
    Serial.println("Connected to MQTT");
    mqttClient->subscribe(MQTT_TOPIC);
  }
}

void mqttCallback(const MQTT::Publish& pub) {
  Serial.print("MQTT message: ");
  Serial.println(pub.payload_string());

  if (pub.payload_string() == "on") {
    turnOnBuzzer();
  }
  if (pub.payload_string() == "off") {
    turnOffBuzzer();
  }

  mqttClient->publish(MQTT::Publish(MQTT_TOPIC"/status", pub.payload_string()).set_retain(0).set_qos(1));
}

void turnOffBuzzer() {
  Serial.println("Turning buzzer off");
  digitalWrite(RELAY, LOW);
  timeBuzzerTurnedOn = 0;
}

void turnOnBuzzer() {
  Serial.println("Turning buzzer on");
    digitalWrite(RELAY, HIGH);
    timeBuzzerTurnedOn = millis();
}

void checkConnection() {
  if (WiFi.status() != WL_CONNECTED)  {
    connectToWifi();
  }
  if (WiFi.status() == WL_CONNECTED && !mqttClient->connected()) {
    connectToMqtt();
  }

  lastConnectionCheckTime = millis();
}

