#include "MqttConnect.h"
MqttConnect *globalMqttConnect = nullptr;

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
  globalMqttConnect = this;
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
  client->setCallback(mqttCallback);
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

bool MqttConnect::connected(){
  client->connected();
}
bool MqttConnect::publish(string topic, string payload){
  if (connected()){
    client->publish(topic.c_str(), payload.c_str());
  }
  else{
    Serial.println("Cannot publish, not connected");
  }
}

bool MqttConnect::publish(string topic, string payload, boolean retained){
  if (connected()){
    client->publish(topic.c_str(), payload.c_str(), retained);
  }
  else{
    Serial.println("Cannot publish, not connected");
  }
}

bool MqttConnect::subscribe(MqttSubscription sub){
  if (sub.qos >=0){
    return client->subscribe(sub.topic.c_str(), sub.qos);
  }
  else{
    return client->subscribe(sub.topic.c_str());
  }
  _subscriptionMap.insert(std::make_pair(sub.topic, sub));
}

bool MqttConnect::subscribe(string topic,void (* callback)(string)){
  MqttSubscription mqttSub;
  mqttSub.topic = topic;
  mqttSub.callback  = callback;
  subscribe(mqttSub);
}

void MqttConnect::mqttCallback(char* topic, byte* payload, unsigned int length){
  char * payloadChar = (char *) malloc((length)*sizeof(char));
  int i;
  for (i=0;i<length;i=i+1){
    payloadChar[i] = payload[i];
  }
  payloadChar[length]='\0';

  free(payload);
  string topicString = topic;

  if(globalMqttConnect->_subscriptionMap.find(topicString) != globalMqttConnect->_subscriptionMap.end()){
    Serial.print("Callback for topic ");
    Serial.println(topicString.c_str());
    globalMqttConnect->_subscriptionMap[topicString].callback((string) payloadChar);
  } else{
    Serial.print("Didn't callback for topic ");
    Serial.println(topicString.c_str());
  }
}

bool MqttConnect::unsubscribe(string topic){
std::map<string, MqttSubscription>::iterator it;
it = _subscriptionMap.find(topic);
if (it!= _subscriptionMap.end()){
  _subscriptionMap.erase(it);
}
  client->unsubscribe(topic.c_str());
}

bool MqttConnect::loop(){
if(connected()){
  client->loop();
} else {
  Serial.println("Trying to connect MQTT");
  connect();
  Serial.println("Finished trying to connect MQTT");
}

}
