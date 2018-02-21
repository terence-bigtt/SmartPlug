#include "MqttConf.h"

MqttConf::MqttConf(){
  init();
}
MqttConf::MqttConf(std::string broker){
  init();
  setBroker(broker);
}
MqttConf::MqttConf(std::string broker, std::string port){
  init();
  setBroker(broker);
  setPort(port);
}
MqttConf::MqttConf(std::string broker, std::string port, std::string id){
  init();
  setBroker(broker);
  setPort(port);
  setId(id);
}
MqttConf::MqttConf(std::string broker, std::string port, std::string id,
  std::string user, std::string pwd){
    init();
    setBroker(broker);
    setPort(port);
    setId(id);
    setUser(user);
    setPassword(pwd);
  }
MqttConf::~MqttConf(){
}

bool MqttConf::init(){
  return SPIFFS.begin();
}
std::string MqttConf::getBroker(){
  return _broker;
}
std::string MqttConf::getPort(){
  return _port;
}
std::string MqttConf::getUser(){
  return _user;
}
std::string MqttConf::getPassword(){
  return _pwd;
}
std::string MqttConf::getId(){
  return _id;
}
void MqttConf::setBroker(std::string broker){
  _broker= broker;
}
void MqttConf::setUser(std::string user){
  _user= user;
}
void MqttConf::setPassword(std::string pwd){
  _pwd= pwd;
}
void MqttConf::setId(std::string id){
  _id =  id;
}
void MqttConf::setPort(std::string port){
  _port =  port;
}

bool MqttConf::valid(){
  Serial.print("Mqtt confifguration is valid: ");
  Serial.println(_broker.length()>0);
  return _broker.length()>0;
}

bool MqttConf::readConfig(){
  if (SPIFFS.exists(MQTT_CONFIG_FILE)) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(MQTT_CONFIG_FILE, "r");
      if (configFile) {
        Serial.println("opened config file");
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(configFile);
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          _broker= (std::string) (json["broker"] | "");
          _port= (std::string) (json["port"] | "");
          _id= (std::string) (json["id"] | "");
          _user= (std::string) (json["user"] | "");
          _pwd= (std::string) (json["pwd"] | "");
          Serial.println("I saved these values:");
          Serial.println(_broker.c_str());
          Serial.println(_port.c_str());
          Serial.println(_id.c_str());
          Serial.println(_user.c_str());
          Serial.println(_pwd.c_str());
        } else {
          Serial.println("failed to load json config");
          return false;
        }
        return true;
      }
    }
    return false;
}

bool MqttConf::writeConfig(){
  Serial.println("I'm gonna save the config to flash.");
  File configFile = SPIFFS.open(MQTT_CONFIG_FILE, "w");
  Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["broker"] = _broker.c_str();
    json["port"] = _port.c_str();
    json["id"] = _id.c_str();
    json["user"] =_user.c_str();
    json["pwd"] = _pwd.c_str();

    if (!configFile) {
      Serial.println("failed to open config file for writing");
      return false;
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    return true;
}
