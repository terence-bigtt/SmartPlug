#ifndef MqttConnect_h
#define MqttConnect_h

#include "MqttConf.h"
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <string>
#include <map>
#include <new>

using namespace std;

class MqttConnect{
public:
  struct MqttSubscription{
      string topic;
      void (*callback)(string);// callback(payload)
      uint8_t qos = -1;
  };
  MqttConnect();
  MqttConnect(MqttConf * p_conf);
  ~MqttConnect();
  void setConfig(MqttConf * p_conf);
  bool connect();
  bool connected();
  bool publish(string topic, string payload);
  bool publish(string topic, string payload, boolean retained);

  bool subscribe(MqttSubscription sub);
  bool subscribe(string topic, void (* callback)(string));

  bool unsubscribe(string topic);
  bool loop();
  PubSubClient *client;

private:
  std::map<string, MqttSubscription> _subscriptionMap;
  WiFiClient espClient;
  MqttConf * _conf;
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
  void initialize();
};
#endif
