#ifndef MqttConnect_h
#define MqttConnect_h

#include "MqttConf.h"
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <string>
#include <new>

using namespace std;

class MqttConnect{
public:
  MqttConnect();
  MqttConnect(MqttConf * p_conf);
  ~MqttConnect();
  void setConfig(MqttConf * p_conf);
  bool connect();
  bool connected();
  bool publish(string topic, string payload);
  bool publish(string topic, string payload, boolean retained);
  bool subscribe(string topic);
  bool subscribe(string topic, uint8_t qos);
  bool unsubscribe(string topic);
  bool loop();
  bool connect(void (* callback)(char* topic, byte* payload, unsigned int length) );
  void setCallback(void (* callback)(char* topic, byte* payload, unsigned int length) );
  PubSubClient *client;
  void heartBeat();
  void setBeatInterval(int interval);
  void setHeartBeatTopic(string topic);

private:
  WiFiClient espClient;
  MqttConf * _conf;
  int _beatInterval = 30000;
  long _lastBeat = 0;
  string _heartBeatTopic= "/heartbeat";
  void (* _callback)(char* topic, byte* payload, unsigned int length) =NULL;
  void initialize();
};
#endif
