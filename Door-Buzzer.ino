#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_SSID "xxxx"
#define WIFI_PASS "xxxx"
#include "Webserver.h"

#define MQTT_CLIENT "Door-Buzzer"
#define MQTT_SERVER "192.168.1.157"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "home/doorbuzzer"

#define RELAY 4
#define BUZZER_ON_TIME 20 // In seconds
#define CONNECTION_CHECK_INTERVAL 10 // In seconds

extern "C" { 
  #include "user_interface.h" 
}

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient, MQTT_SERVER, MQTT_PORT);
Webserver *webserver;
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
  
  mqttClient.set_callback(mqttCallback);
  webserver = new Webserver;
  if (connectToWifi()) {
    connectToMqtt();
  }
}

void loop() {
  mqttClient.loop();

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
  WiFi.begin(WIFI_SSID, WIFI_PASS);
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

  while (!mqttClient.connect(MQTT::Connect(MQTT_CLIENT).set_keepalive(90)) && retries--) {
    Serial.print(" .");
    delay(1000);
  }

  if(mqttClient.connected()) {
    Serial.println("Connected to MQTT");
    mqttClient.subscribe(MQTT_TOPIC);
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

  mqttClient.publish(MQTT::Publish(MQTT_TOPIC"/status", pub.payload_string()).set_retain(0).set_qos(1));
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
  if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
    connectToMqtt();
  }

  lastConnectionCheckTime = millis();
}

