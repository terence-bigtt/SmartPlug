#include "MqttConnect.h"

MqttConnect::MqttConnect(){
  initialize();
}

MqttConnect::MqttConnect(MqttConf * p_conf){
  initialize();
  setConfig(p_conf);
}

MqttConnect::~MqttConnect(){
  delete client;
}

void MqttConnect::initialize(){
  client = new PubSubClient(espClient);
}

void MqttConnect::setConfig(MqttConf * p_conf){
  _conf = p_conf;
}

bool MqttConnect::connect(){
  bool connected;
  string broker = _conf->getBroker();
  string port = _conf->getPort();
  string id = _conf->getId();
  string user = _conf->getUser();
  string pwd = _conf->getPassword();

  client->setServer(broker.c_str(), atoi(port.c_str()));
  if(id.length()==0){
    id = "ESP";
  }
  client->setCallback(_callback);
  Serial.println("will attempt connection.");
  Serial.println(user.c_str());

  if(user.length()!=0){
    connected= client->connect(id.c_str(), user.c_str(), pwd.c_str());
  }
  else {
    connected= client->connect(id.c_str());
  }
  Serial.print("MC:: Mqtt connected: ");
  Serial.println(connected);
  return connected;
}

bool MqttConnect::connect(void (*callback)(char* topic, byte* payload, unsigned int length)){
  setCallback(*callback);
  return connect();
}

bool MqttConnect::connected(){
  client->connected();
}
bool MqttConnect::publish(string topic, string payload){
  client->publish(topic.c_str(), payload.c_str());
}

bool MqttConnect::publish(string topic, string payload, boolean retained){
  client->publish(topic.c_str(), payload.c_str(), retained);
}

bool MqttConnect::subscribe(string topic){

  if(client->subscribe(topic.c_str())){
    Serial.print("Subscribed to ");
    Serial.println(topic.c_str());
  }
  else {
    Serial.println("Couldn't subscribe");
  }

}

bool MqttConnect::subscribe(string topic, uint8_t qos){
  client->subscribe(topic.c_str(), qos);
}
bool MqttConnect::unsubscribe(string topic){
  client->unsubscribe(topic.c_str());
}
bool MqttConnect::loop(){
  client->loop();
  heartBeat();
}

void MqttConnect::setCallback(void (* callback)(char* topic, byte* payload, unsigned int length) ){
  _callback = callback;
}

void MqttConnect::setHeartBeatTopic(string topic){
  _heartBeatTopic = topic;
}

void MqttConnect::heartBeat(){
  long now = millis();

  if (now-_lastBeat>_beatInterval){
    _lastBeat = now;
    char buf[50];
    sprintf(buf, "HeartBeat - ESP%d - %ul", ESP.getChipId(), now);
    string beatMsg = (string) buf;
    Serial.println(beatMsg.c_str());
    if (client->connected()){
      client->publish(_heartBeatTopic.c_str(), beatMsg.c_str());
    }
  }
}

void MqttConnect::setBeatInterval(int interval){

}
