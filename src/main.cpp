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
#include <ESP8266mDNS.h>


//#include "MDns/mDns.h"
#include <string>
#include <sstream>

#define SWITCH_PIN 5
#define APMODE_PIN 2
#define CLIMODE_PIN 0
#define MANUAL_PIN 4

#define SWITCH_ON 0
#define SWITCH_OFF 1
#define HEARTBEAT 30000

#define MQTT_CLIENT_ID "ESP-001"
#define SWITCH_ACTION_TOPIC "switch/carport/light/control"
#define SWITCH_STATUS_TOPIC "switch/carport/light/status"
#define SERIAL_BAUD 115200

bool saveNewConfig = false;
unsigned long lastBeat = 0;

ConfigButton configButton(MANUAL_PIN, HIGH);
MqttConf mqttConf;
MqttConnect mqtt(&mqttConf);
ESP8266WebServer webServer(80);

int _currentState= LOW;

char mqtt_server[40] = "";
char mqtt_port[6] = "1883";
char mqtt_id[10] ="ESP-01";

char mqtt_user[20] = "terence";
char mqtt_pwd[20] = "";

char networkName[20]="esp_01";

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
  //mdns.setName((std::string) networkName);
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
  WiFiManagerParameter mdnsName("password", "mqtt password", networkName, 20);
  wifiManager.addParameter(&server);
  wifiManager.addParameter(&port);
  wifiManager.addParameter(&id);
  wifiManager.addParameter(&user);
  wifiManager.addParameter(&pwd);
  wifiManager.addParameter(&mdnsName);
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
    strcpy(networkName, mdnsName.getValue());
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
}
void switchOff(){
	log("Switching off");
	digitalWrite(SWITCH_PIN, HIGH);
}

void mqttSwitchTopicHandler(string payload){
  Serial.print("Received message ");

    if(payload == "on"){
        switchOn();
    }
    if(payload == "off"){
        switchOff();
    }
    if(payload == "restart"){
        restart();
    }
}

bool configMqtt(){
  mqtt.subscribe((string) SWITCH_ACTION_TOPIC, mqttSwitchTopicHandler);
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
  if (!mqtt.connect()){
    Serial.println("couldn't connect to MQTT server.");
  }
  else{
    configMqtt();
  }
  if(MDNS.begin(((std::string) networkName).c_str())){
    Serial.println("MDNS responder started");
    MDNS.addService("esp", "http", 80);
  }
  Serial.println("Setting up server for /on.");
  webServer.on("/on",[](){switchOn(); webServer.send(200, "text/plain", getSwitchStateAsString().c_str());});
  Serial.println("Setting up server for /off.");
  webServer.on("/off",[](){switchOff(); webServer.send(200, "text/plain", getSwitchStateAsString().c_str());});
  webServer.begin();
  Serial.println("HTTP server started");
  Serial.println("Setting up server for /toggle.");
  webServer.on("/toggle",[](){switchToggle(); webServer.send(200, "text/plain", getSwitchStateAsString().c_str());});
  Serial.println("Setting up server for /status.");
  webServer.on("/status",[](){webServer.send(200, "text/plain", getSwitchStateAsString().c_str());});
  Serial.println("Server set up done");


}

void loop()
{ int liveState= digitalRead(SWITCH_PIN);
  configButton.loop();
  webServer.handleClient();
  mqtt.loop();
	if(liveState != _currentState) {
    Serial.println("State changed");
		mqtt.publish(SWITCH_STATUS_TOPIC, getSwitchStateAsString().c_str(), true);
		_currentState = liveState;
  }
}
