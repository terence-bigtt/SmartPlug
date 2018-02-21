#include "ConfigButton/ConfigButton.h"
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include "Mqtt/MqttConf.h"
#include "Mqtt/MqttConnect.h"
#include <string>
#include <sstream>

#define SWITCH_PIN 5
#define APMODE_PIN 0
#define CLIMODE_PIN 2
#define MANUAL_PIN 4

#define SWITCH_ON 0
#define SWITCH_OFF 1

#define MQTT_CLIENT_ID "ESP-001"
#define SWITCH_ACTION_TOPIC "switch/carport/light/control"
#define SWITCH_STATUS_TOPIC "switch/carport/light/status"
#define SERIAL_BAUD 115200

bool saveNewConfig = false;

void mqttCallbackHandler(char *topic, byte *payload, unsigned int length);

ConfigButton configButton(MANUAL_PIN, HIGH);
MqttConf mqttConf;
MqttConnect mqtt(&mqttConf);


int _currentState= LOW;

char mqtt_server[40] = "";
char mqtt_port[6] = "1883";
char mqtt_id[10] ="ESP-01";

char mqtt_user[20] = "terence";
char mqtt_pwd[20] = "";

void saveConfigCallback(){
  saveNewConfig = true;
}

void saveConfig(){
  Serial.println("I'm gonna call the writing function.");
  Serial.print("I read this broker value: ");
  Serial.println(mqtt_server);

  mqttConf.setBroker((std::string) mqtt_server);
  mqttConf.setPort((std::string) mqtt_port);
  mqttConf.setId((std::string) mqtt_id);
  mqttConf.setUser((std::string) mqtt_user);
  mqttConf.setPassword((std::string) mqtt_pwd);
  if(mqttConf.writeConfig()){
    Serial.println("Saved config");
  }
  else{
    Serial.println("Failed saving config.");
  }
  saveNewConfig=false;
}

void feedbackApMode(){
  digitalWrite(APMODE_PIN, HIGH);
  digitalWrite(CLIMODE_PIN, LOW);
}

void feedbackStaMode(){
  digitalWrite(APMODE_PIN, LOW);
  digitalWrite(CLIMODE_PIN, HIGH);
}

void runConfigPortal(){
  saveNewConfig = false;
  WiFiManager wifiManager;
  WiFiManagerParameter server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter id("id", "mqtt id", mqtt_id, 10);
  WiFiManagerParameter user("user", "mqtt user", mqtt_user, 20);
  WiFiManagerParameter pwd("password", "mqtt password", mqtt_pwd, 20);
  wifiManager.addParameter(&server);
  wifiManager.addParameter(&port);
  wifiManager.addParameter(&id);
  wifiManager.addParameter(&user);
  wifiManager.addParameter(&pwd);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  feedbackApMode();
  if (!wifiManager.startConfigPortal("OnDemandAP")) {
    Serial.println("failed to connect and hit timeout");
  }
  feedbackStaMode();
  Serial.println("connected...yeey :)");
  if (saveNewConfig){
    strcpy(mqtt_server, server.getValue());
    strcpy(mqtt_port, port.getValue());
    strcpy(mqtt_id, id.getValue());
    strcpy(mqtt_user, user.getValue());
    strcpy(mqtt_pwd, pwd.getValue());
    saveConfig();
  }
}

std::string getSwitchStateAsString(){
  switch (digitalRead(SWITCH_PIN)) {
    case LOW: return "on";
    case HIGH: return "off";
  }
}

void restart(){
  ESP.restart();
}

void switchToggle(){
  int current = digitalRead(SWITCH_PIN);
  log("toggling switch");
  digitalWrite(SWITCH_PIN, !current);
}
void switchOn(){
	log("Switching on");
	digitalWrite(SWITCH_PIN, LOW);
  digitalWrite(2,HIGH);
}
void switchOff(){
	log("Switching off");
	digitalWrite(SWITCH_PIN, HIGH);
  digitalWrite(2,LOW);
}

bool configMqtt(){
  mqtt.subscribe(SWITCH_ACTION_TOPIC);
  Serial.println("Subscribed to Action Topic");
  mqtt.publish(SWITCH_STATUS_TOPIC, getSwitchStateAsString().c_str(), true);
}

void setup(){
  Serial.begin(115200);
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(APMODE_PIN, OUTPUT);
  pinMode(CLIMODE_PIN, OUTPUT);
  digitalWrite(SWITCH_PIN, HIGH);
  digitalWrite(APMODE_PIN, HIGH);
  digitalWrite(CLIMODE_PIN, HIGH);

  configButton.setClickCallback(1, switchToggle);
  configButton.setClickCallback(5, restart);
  configButton.setPressCallback(1, runConfigPortal);

  bool hasConfig = mqttConf.readConfig();
  bool configValid = mqttConf.valid();
  if (!hasConfig || !configValid){
    Serial.println("No config detected or invalid config.");
    runConfigPortal();
    return;
  }

  WiFiManager wifiManager;
  if(!wifiManager.autoConnect()){
    Serial.println("Autoconnect failed. will restart");
    ESP.reset();
  };
  feedbackStaMode();
  if (!mqtt.connect(mqttCallbackHandler)){
    Serial.println("couldn't connect to MQTT server.");
  }
  else{
    configMqtt();
  }

}

void loop()
{ int liveState= digitalRead(SWITCH_PIN);
  configButton.loop();
  if(mqtt.connected()){
    mqtt.loop();

		if(liveState != _currentState) {
      Serial.println("State changed");
			mqtt.publish(SWITCH_STATUS_TOPIC, getSwitchStateAsString().c_str(), true);
			_currentState = liveState;
    }
  } else {
    Serial.println("Trying to connect MQTT");
    mqtt.connect();
    Serial.println("Finished trying to connect MQTT");
  }

}

void mqttCallbackHandler(char *topic, byte *payload, unsigned int length){
  Serial.print("Receied message ");

  char * payloadStr = (char *) malloc((length)*sizeof(char));
  int i;
  for (i=0;i<length;i=i+1){
    payloadStr[i] = payload[i];
  }
  payloadStr[length]='\0';
  Serial.print("Receied message ");
  Serial.print(((std::string) payloadStr).c_str());
  Serial.print(" on topic ");
  Serial.println(((std::string) topic).c_str());


	if(strcmp(topic, SWITCH_ACTION_TOPIC) == 0) {
    if(strcmp(payloadStr, "on")==0){
        switchOn();
    }
    if(strcmp(payloadStr, "off")==0){
        switchOff();
    }
    if(strcmp(payloadStr, "restart")==0){
        restart();
    }
  }
  free(payloadStr);
}
