#ifndef MqttConf_h
#define MqttConf_h
#include <FS.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <algorithm>    // std::copy
#include <string> //std::string
#define MQTT_CONFIG_FILE "/mqtt.json"

class MqttConf{
public:
  MqttConf();
  MqttConf(std::string broker);
  MqttConf(std::string broker, std::string port);
  MqttConf(std::string broker, std::string port, std::string id);
  MqttConf(std::string broker, std::string port, std::string id,
    std::string user, std::string pwd);
  ~MqttConf();

  std::string getBroker();
  std::string getPort();
  std::string getUser();
  std::string getPassword();
  std::string getId();
  void setBroker(std::string broker);
  void setUser( std::string user);
  void setPassword( std::string pwd);
  void setId(std::string id);
  void setPort(std::string port);
  bool valid();
  bool readConfig();
  bool writeConfig();
private:
  std::string _broker="";
  std::string _user="";
  std::string _pwd="";
  std::string _id="";
  std::string _port="8133";
  bool init();
};

#endif
