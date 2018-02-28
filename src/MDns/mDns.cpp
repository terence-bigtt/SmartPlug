#include "mDns.h"

MdnsResponder::MdnsResponder(string networkName, int port){
setPort(port);
setName(networkName);
}

bool MdnsResponder::setup(){
  Serial.println("begining MDNS");
  _mdnsUp = MDNS.begin(_name.c_str());
  Serial.print("MDNS UP: ");
  Serial.println(_mdnsUp);
  if(_mdnsUp){
    Serial.println("Add service");
    MDNS.addService("http", "tcp", _port);
    Serial.println("Server reset");
    server.reset(new ESP8266WebServer(_port));
    Serial.println("Server begin");
    server->begin();
    Serial.println("Server reset");

  }
  return _mdnsUp;
}

bool MdnsResponder::isUp(){
  return _mdnsUp;
}
void MdnsResponder::loop(){
  if(_mdnsUp){
    server->handleClient();
  }
}
